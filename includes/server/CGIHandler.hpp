// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<CGIHandler.hpp>> -- <<Aida, Ilmari, Milica>>

#pragma once

#include "http/Request.hpp"
#include "server/EventHandler.hpp"

class Client;

typedef std::unordered_map<i32 *, std::vector<EventHandler *>>	fdMap;

class CGIHandler: public EventHandler {
	private:
		std::vector<const char *>	_envp;
		std::vector<std::string>	_env;

		const ServerBlock	*_serverBlock;

		std::string	_scriptName;
		std::string	_scriptPath;
		std::string	_absPath;
		std::string	_query;
		std::string	_buf;

		Client	*_client;

		pid_t	_pid;

		struct {
			struct epoll_event	event;
			i32					pfd[2];
		}	_outputPipe;

		struct {
			struct epoll_event	event;
			i32					pfd[2];
		}	_inputPipe;

		enum {
			GET,
			POST
		}	_method;

		i32	&_socketFd;
		i32	&_epollFd;

		// private methods
		bool	_setupPipes(void);
		bool	_setupEnv(const Request &req);
		bool	_done(void) const;

		i32	_write(void);
		i32	_read(void);

	public:
		CGIHandler(void) = delete;
		CGIHandler(Client *client, i32 &sfd, i32 &efd);
		~CGIHandler(void);

		CGIHandler(const CGIHandler &other) = delete;
		CGIHandler	&operator=(const CGIHandler &other) = delete;

		CGIHandler(CGIHandler &&other) noexcept;
		CGIHandler	&operator=(const CGIHandler &&other) noexcept;

		// public methods
		bool	init(const Request &req);
		bool	exec(fdMap &activeFds);

		void	resolveClose(void) override;

		i32	handleEvent(const u32 ev, i32 &efd) override;

		// public setters
		void	setServerBlock(const ServerBlock *sb);
		void	setClient(Client *client);

		// public getters
		const struct epoll_event	&getOutputEvent(void) const;
		const struct epoll_event	&getInputEvent(void) const;

		const i32	&getOutputFd(void) const;
		const i32	&getInputFd(void) const;

		Client	*getClient(void) const;

		// fuck inheritance
		std::vector<EventHandler *>	resolveAccept(void) override;

		struct epoll_event	&getCgiEvent(int flag) override;

		EventHandler	*getCgi(void) override;

		bool	conditionMet(fdMap &activeFds, i32 &epollFd) override;

		void	setErrorCgi(void) override;

		i32	*getSocketFd(const i32 flag) override;

		i32	ready2Switch(void) override;
};
