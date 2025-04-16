// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<WebServer.hpp>> -- <<Aida, Ilmari, Milica>>

#pragma once

// #include "EventLoop.hpp"
#include "CommonFunctions.hpp"
#include "server/Listener.hpp"
#include "VirtualHost.hpp"
#include "config/ServerBlock.hpp"

#define TRUE 1
#define FALSE 0

class	WebServer{
	private:
		std::unordered_map<int*, std::vector<VirtualHost>> _theVHList;
		std::unordered_map<std::string, std::vector<std::string>> _theSList;
		std::vector<Listener>       _theLList;
		// std::vector<VirtualHost>	_virtualHosts;
		// std::vector<int>			_fds;
		// EventLoop					_eventLoop;
		WebServer obj(const WebServer& other) = delete;
		WebServer& operator=(const WebServer& other) = delete;
	public:
		WebServer();
		~WebServer();

		int	initialize(std::vector<ServerBlock>& serBlocks); //create listening and virtual hosts, set them
		// void	run(void); //epoll + accepting connections + event handling
		void freeStuff(void);

		std::vector<Listener>& getListeners(void);
		std::vector<VirtualHost> getVHosts(void) const;
		bool doesExist(std::string port, std::string host);
		bool doesExistPort(std::string port);
		int resolveListener(std::string port, std::string host);
};

// void ftMemset(void *dest, std::size_t count);
