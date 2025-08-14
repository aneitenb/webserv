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
#include "config/ServerBlock.hpp"
#include "server/EventHandler.hpp"

class Client;

typedef std::pair<std::string, LocationBlock>	CGILocation;

typedef std::unordered_map<i32 *, std::vector<EventHandler *>>	fdMap;

class CGIHandler: public EventHandler {
	private:
		std::vector<const char *>	_envp;
		std::vector<std::string>	_env;

		const CGILocation	*_location;
		const ServerBlock	*_serverBlock;

		std::string	_scriptFile;
		std::string	_scriptDir;
		std::string	_pathInfo;
		std::string	_query;
		std::string	_buf;

		Client	*_client;

		pid_t	_pid;

		bool	_valid;

		struct {
			struct epoll_event	event;
			i32					pfd[2];
		}	_outputPipe;

		struct {
			struct epoll_event	event;
			i32					pfd[2];
		}	_inputPipe;

		enum {
			GET = 1,
			POST = 2,
			DELETE = 4
		}	_method;

		i32	&_socketFd;
		i32	&_epollFd;

		u16	_errorCode;

		// private methods
		void	_setupPipes(void);
		bool	_setupEnv(const Request &req);

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
		void	resolveClose(void) override;
		void	stop(void);
		bool	init(const Request &req);
		bool	exec(fdMap &activeFds);

		i32	handleEvent(const u32 ev, i32 &efd) override;

		// public setters
		void	setServerBlock(const ServerBlock *sb);
		void	setLocation(const CGILocation *loc);
		void	setClient(Client *client);

		// public getters
		const std::string	&getResponseData(void) const;

		const bool	&isValid(void) const;

		const u16	&getErrorCode(void) const;

		Client	*getClient(void) const;

		// fuck inheritance
		std::vector<EventHandler *>	resolveAccept(void) override;

		struct epoll_event	&getCgiEvent(int flag) override;

		EventHandler	*getCgi(void) override;

		bool	conditionMet(fdMap &activeFds, i32 &epollFd) override;

		i32	*getSocketFd(const i32 flag) override;

		i32	ready2Switch(void) override;
};
