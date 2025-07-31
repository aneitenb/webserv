// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<CGIHandler.hpp>> -- <<Aida, Ilmari, Milica>>

#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

#include "utils/message.hpp"
#include "server/Client.hpp"
#include "server/CGIHandler.hpp"

#undef Info
#define Info(msg)	(info(std::stringstream("") << msg, COLOR_CGI))

#define _PID_ERROR	-1
#define _PID_CHILD	0

#define _BUF_SIZE	4096

#define makeVar(var, val)	(std::string(std::string(var) + "=" + val))

static inline bool	_pipe(i32 (*pfd)[2]);

CGIHandler::CGIHandler(Client *client, i32 &sfd, i32 &efd): _client(client), _method(CGIHandler::GET), _socketFd(sfd), _epollFd(efd) {
	this->_outputPipe.event.events = EPOLLIN;
	this->_inputPipe.event.events = EPOLLOUT;
	this->_outputPipe.pfd[0] = -1;
	this->_outputPipe.pfd[1] = -1;
	this->_inputPipe.pfd[0] = -1;
	this->_inputPipe.pfd[1] = -1;
	this->_location = nullptr;
	this->setState(FORCGI);
}

CGIHandler::~CGIHandler(void) {}

CGIHandler::CGIHandler(CGIHandler &&other) noexcept: _location(other._location), _socketFd(other._socketFd), _epollFd(other._epollFd) {
	*this = std::move(other);
}

CGIHandler	&CGIHandler::operator=(const CGIHandler &&other) noexcept {
	if (this != &other) {
		this->resolveClose();
		this->_envp = other._envp;
		this->_env = other._env;
		this->_location = other._location;
		this->_serverBlock = other._serverBlock;
		this->_scriptFile = other._scriptFile;
		this->_scriptDir = other._scriptDir;
		this->_pathInfo = other._pathInfo;
		this->_query = other._query;
		this->_buf = other._buf;
		this->_client = other._client;
		this->_pid = other._pid;
		this->_valid = other._valid;
		this->_outputPipe.event.events = other._outputPipe.event.events;
		this->_inputPipe.event.events = other._inputPipe.event.events;
		this->_outputPipe.pfd[0] = other._outputPipe.pfd[0];
		this->_outputPipe.pfd[1] = other._outputPipe.pfd[1];
		this->_inputPipe.pfd[0] = other._inputPipe.pfd[0];
		this->_inputPipe.pfd[1] = other._inputPipe.pfd[1];
		this->_method = other._method;
		this->_socketFd = other._socketFd;
		this->_epollFd = other._epollFd;
		this->_errorCode = other._errorCode;
		this->setState(other.getState());
	}
	return *this;
}

// private methods
void	CGIHandler::_setupPipes(void) {
	this->_outputPipe.event.data.ptr = static_cast<void *>(this);
	this->_inputPipe.event.data.ptr = static_cast<void *>(this);
	if (!_pipe(&this->_outputPipe.pfd)) {
		this->_errorCode = HTTP_INTERNAL_SERVER_ERROR;
		this->_valid = false;
	}
	if (this->_valid && this->_method == CGIHandler::POST) {
		if (!_pipe(&this->_inputPipe.pfd)) {
			closeFd(&this->_outputPipe.pfd[0]);
			closeFd(&this->_outputPipe.pfd[1]);
			this->_errorCode = HTTP_INTERNAL_SERVER_ERROR;
			this->_valid = false;
		}
		if (this->_valid && epoll_ctl(this->_epollFd, EPOLL_CTL_ADD, this->_inputPipe.pfd[1], &this->_inputPipe.event) == -1) {
			Warn("CGIHandler::_setupPipes(): epoll_ctl(" << this->_epollFd << ", EPOLL_CTL_ADD, " << this->_inputPipe.pfd[1]
				 << ", &this->_inputPipe.event) failed: " << strerror(errno));
			this->_client->setState(CLOSE);
			this->resolveClose();
			this->_errorCode = HTTP_INTERNAL_SERVER_ERROR;
			this->_valid = false;
		}
	} else if (this->_valid && epoll_ctl(this->_epollFd, EPOLL_CTL_ADD, this->_outputPipe.pfd[0], &this->_outputPipe.event) == -1) {
		Warn("CGIHandler::_setupPipes(): epoll_ctl(" << this->_epollFd << ", EPOLL_CTL_ADD, " << this->_outputPipe.pfd[0]
				<< ", &this->_outputPipe.event) failed: " << strerror(errno));
		this->_client->setState(CLOSE);
		this->resolveClose();
		this->_errorCode = HTTP_INTERNAL_SERVER_ERROR;
		this->_valid = false;
	}
}

bool	CGIHandler::_setupEnv(const Request &req) {
	std::string	serverName;
	std::string	serverPort;
	std::string	absPath;
	std::string	root;
	size_t		i;

	this->_env.clear();
	this->_envp.clear();
	root = _serverBlock->getRoot();
	absPath = root + this->_location->first;
	if (access(absPath.c_str(), F_OK) == -1) {
		Warn("CGIHandler::_setupEnv(): access(" << absPath << ", F_OK) failed: " << strerror(errno));
		this->_errorCode = HTTP_NOT_FOUND;
		this->_valid = false;
		return false;
	}
	if (access(absPath.c_str(), X_OK) == -1) {
		Warn("CGIHandler::_setupEnv(): access(" << absPath << ", X_OK) failed: " << strerror(errno));
		this->_errorCode = HTTP_FORBIDDEN;
		this->_valid = false;
		return false;
	}
	this->_scriptFile = std::string(this->_location->first).erase(0, this->_location->first.find_last_of('/') + 1);
	this->_scriptDir = absPath.erase(absPath.find_last_of('/'));
	this->_pathInfo = req.getURI();
	this->_pathInfo.erase(0, this->_location->first.length());
	i = this->_pathInfo.find_first_of('?');
	if (i != std::string::npos) {
		this->_query = this->_pathInfo.substr(i + 1);
		this->_pathInfo.erase(i);
	}
	if (absPath.front() != '/')
		absPath = std::filesystem::current_path().string() + '/' + absPath;
	if (this->_method == CGIHandler::POST) {
		this->_env.push_back(makeVar("CONTENT_LENGTH", std::to_string(req.getContentLength())));
		this->_env.push_back(makeVar("CONTENT_TYPE", req.getContentType()));
	}
	this->_env.push_back(makeVar("GATEWAY_INTERFACE", "CGI/1.1"));
	this->_env.push_back(makeVar("PATH_INFO", this->_pathInfo));
	this->_env.push_back(makeVar("PATH_TRANSLATED", absPath + '/' + this->_scriptFile + ((!this->_pathInfo.empty()) ? this->_pathInfo : "")));
	this->_env.push_back(makeVar("QUERY_STRING", ((!this->_query.empty()) ? this->_query : "")));
	this->_env.push_back(makeVar("REMOTE_ADDR", this->getClient()->getPeerIP()));
	this->_env.push_back(makeVar("REMOTE_PORT", this->getClient()->getPeerPort()));
	this->_env.push_back(makeVar("REQUEST_METHOD", req.getMethod()));
	this->_env.push_back(makeVar("SCRIPT_NAME", this->_location->first));
	this->_env.push_back(makeVar("SERVER_NAME", this->getClient()->getLocalIP()));
	this->_env.push_back(makeVar("SERVER_PORT", this->getClient()->getLocalPort()));
	this->_env.push_back(makeVar("SERVER_PROTOCOL", "HTTP/1.1"));
	this->_env.push_back(makeVar("SERVER_SOFTWARE", SERVER_NAME));
	for (std::string &var : this->_env)
		this->_envp.push_back(var.c_str());
	this->_envp.push_back(nullptr);
	return true;
}

bool	CGIHandler::_done(void) const {
	i32		status;

	return (waitpid(this->_pid, &status, WNOHANG) == this->_pid) ? true : false;
}

i32	CGIHandler::_write(void) {
	ssize_t		bytesWritten;
	size_t		remaining;

	Info("\nCGIHandler: Sending POST body for client #" << *this->_client->getSocketFd(0));
	bytesWritten = write(_inputPipe.pfd[1], this->_buf.c_str(), this->_buf.size());
	if (bytesWritten == -1) {
		Warn("CGIHandler::_write(): write(" << _inputPipe.pfd[1] << ", this->_buf, " << this->_buf.size() << ") failed: " << strerror(errno));
		this->_errorCode = HTTP_INTERNAL_SERVER_ERROR;
		this->_valid = false;
		return -1;
	}
	remaining = this->_buf.size() - bytesWritten;
	Info("CGIHandler: Sent " << bytesWritten << " bytes, " << remaining << " byte" << ((remaining != 1) ? 's' : '\0') << " remaining");
	this->_buf.erase(0, bytesWritten);
	if (this->_buf.size() == 0) {
		if (epoll_ctl(this->_epollFd, EPOLL_CTL_DEL, this->_inputPipe.pfd[1], 0) == -1) {
			Warn("CGIHandler::_write(): epoll_ctl(" << this->_epollFd << ", EPOLL_CTL_DEL, "
				 << this->_inputPipe.pfd[1] << ", 0) failed: " << strerror(errno));
			this->_errorCode = HTTP_INTERNAL_SERVER_ERROR;
			this->_client->setState(CLOSE);
			this->resolveClose();
			this->_valid = false;
			return -1;
		}
		if (epoll_ctl(this->_epollFd, EPOLL_CTL_ADD, this->_outputPipe.pfd[0], &this->_outputPipe.event) == -1) {
			Warn("CGIHandler::_write(): epoll_ctl(" << this->_epollFd << ", EPOLL_CTL_ADD, " << this->_outputPipe.pfd[0]
				 << ", &this->_outputPipe.event) failed: " << strerror(errno));
			this->_errorCode = HTTP_INTERNAL_SERVER_ERROR;
			this->_client->setState(CLOSE);
			this->resolveClose();
			this->_valid = false;
			return -1;
		}
	}
	return 0;
}

i32	CGIHandler::_read(void) {
	std::string	buf;
	ssize_t		bytesRead;

	Info("\nCGIHandler: Reading response for client #" << *this->_client->getSocketFd(0));
	buf.resize(_BUF_SIZE);
	bytesRead = read(this->_outputPipe.pfd[0], &buf[0], _BUF_SIZE);
	if (bytesRead == -1) {
		Warn("CGIHandler::_read(): read(" << this->_outputPipe.pfd[0] << ", buf, "
				<< _BUF_SIZE << ") failed: " << strerror(errno));
		this->_errorCode = HTTP_INTERNAL_SERVER_ERROR;
		this->_client->setState(CLOSE);
		this->resolveClose();
		this->_valid = false;
		return -1;
	}
	if (bytesRead > 0) {
		buf.resize(bytesRead);
		this->_buf.append(buf);
	}
	Info("CGIHandler: Read " << bytesRead  << " bytes, total response length now " << this->_buf.size() << " bytes");
	if (bytesRead == 0 || this->_done()) {
		if (epoll_ctl(this->_epollFd, EPOLL_CTL_DEL, this->_outputPipe.pfd[0], 0) == -1) {
			Warn("CGIHandler::_read(): epoll_ctl(" << this->_epollFd << ", EPOLL_CTL_DEL, "
					<< this->_outputPipe.pfd[0] << ", 0) failed: " << strerror(errno));
			this->_errorCode = HTTP_INTERNAL_SERVER_ERROR;
			this->_client->setState(CLOSE);
			this->resolveClose();
			this->_valid = false;
			return -1;
		}
		if (this->_buf.size() == 0) {
			this->_errorCode = HTTP_INTERNAL_SERVER_ERROR;
			this->_valid = false;
		}
		this->setState(CGIWRITE);
		this->resolveClose();
	}
	return 0;
}

static inline bool	_pipe(i32 (*pfd)[2]) {
	if (pipe(*pfd) == -1) {
		Warn("CGIHandler::_setupPipes(): _pipe: pipe failed: " << strerror(errno));
		return false;
	}
	return true;
}

// public methods
bool	CGIHandler::init(const Request &req) {
	Info("\nCGIHandler: Init started for client #" << *this->_client->getSocketFd(0));
	this->_scriptFile.clear();
	this->_scriptDir.clear();
	this->_pathInfo.clear();
	this->_query.clear();
	this->_buf.clear();
	this->_valid = true;
	this->setState(FORCGI);
	if (req.getMethod() == "DELETE") {
		this->_errorCode = HTTP_METHOD_NOT_ALLOWED;
		this->_valid = false;
	} else {
		if (req.getMethod() == "POST") {
			this->_method = CGIHandler::POST;
			this->_buf = req.getBody();
		} else
			this->_method = CGIHandler::GET;
		if (this->_setupEnv(req))
			this->_setupPipes();
	}
	if (this->_valid)
		return true;
	this->setState(CGIWRITE);
	return false;
}

bool	CGIHandler::exec(fdMap &activeFds) {
	std::string	CGIPass;
	const char	*av[3];
	size_t		i;

	this->_pid = fork();
	switch (_pid) {
		case _PID_ERROR:
			this->_errorCode = HTTP_INTERNAL_SERVER_ERROR;
			this->setState(CGIWRITE);
			this->_valid = false;
			return false;
		case _PID_CHILD:
			for (auto &pair : activeFds) {
				for (i = 0; i < pair.second.size(); ++i)
					closeFd(pair.second.at(i)->getSocketFd(0));
				closeFd(pair.first);
			}
			closeFd(&this->_epollFd);
			if (this->_method == CGIHandler::POST && dup2(this->_inputPipe.pfd[0], 0) == -1) {
				Warn("CGIHandler::_exec: dup2(" << this->_inputPipe.pfd[0] << ", 0) failed: " << strerror(errno));
				_exit(1);
			}
			if (dup2(this->_outputPipe.pfd[1], 1) == -1) {
				Warn("CGIHandler::_exec: dup2(" << this->_outputPipe.pfd[1] << ", 1) failed: " << strerror(errno));
				_exit(1);
			}
			if (chdir(this->_scriptDir.c_str()) != 0) {
				Warn("CGIHandler::_exec: chdir(" << this->_scriptDir.c_str() << ") failed: " << strerror(errno));
				_exit(1);
			}
			CGIPass = this->_location->second.getCgiPass();
			av[0] = CGIPass.c_str();
			av[1] = this->_scriptFile.c_str();
			av[2] = nullptr;
			this->resolveClose();
			execve(av[0], const_cast<char * const *>(av), const_cast<char * const *>(this->_envp.data()));
			Warn("CGIHandler::_exec: execve(" << av[0] << av << this->_env.data() << ") failed: " << strerror(errno));
			_exit(1);
		default:
			closeFd(&this->_outputPipe.pfd[1]);
			closeFd(&this->_inputPipe.pfd[0]);
	}
	return true;
}

void	CGIHandler::resolveClose(void) {
	closeFd(&this->_outputPipe.pfd[0]);
	closeFd(&this->_outputPipe.pfd[1]);
	closeFd(&this->_inputPipe.pfd[0]);
	closeFd(&this->_inputPipe.pfd[1]);
	if (this->_location) {
		delete this->_location;
		this->_location = nullptr;
	}
}

void	CGIHandler::stop(void) {
	if (this->_pid) {
		kill(this->_pid, SIGKILL);
		this->_pid = 0;
	}
}

i32	CGIHandler::handleEvent(const u32 ev, [[maybe_unused]] i32 &efd) {
	return (ev & EPOLLOUT) ? this->_write() : this->_read();
}

// public setters
void	CGIHandler::setServerBlock(const ServerBlock *sb) { this->_serverBlock = sb; }

void	CGIHandler::setLocation(const CGILocation *loc) {
	if (this->_location)
		delete this->_location;
	this->_location = loc;
}

void	CGIHandler::setClient(Client *client) { this->_client = client; }

// public getters
const std::string	&CGIHandler::getResponseData(void) const { return this->_buf; }

const bool	&CGIHandler::isValid(void) const { return this->_valid; }

const u16	&CGIHandler::getErrorCode(void) const { return this->_errorCode; }

Client	*CGIHandler::getClient(void) const { return this->_client; }

// fuck inheritance
std::vector<EventHandler *>	CGIHandler::resolveAccept(void) { return {}; }

struct epoll_event	&CGIHandler::getCgiEvent([[maybe_unused]] i32 flag) { return this->_outputPipe.event; }

EventHandler	*CGIHandler::getCgi(void) { return nullptr; }

bool	CGIHandler::conditionMet([[maybe_unused]] fdMap &activeFds, [[maybe_unused]] i32 &epollFd) { return false; }

i32	*CGIHandler::getSocketFd([[maybe_unused]] const i32 flag) { return &this->_socketFd; }

i32	CGIHandler::ready2Switch(void) { return 42; }
