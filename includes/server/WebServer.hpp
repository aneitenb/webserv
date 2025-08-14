// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<WebServer.hpp>> -- <<Aida, Ilmari, Milica>>

#pragma once

#include "CommonFunctions.hpp"
#include "server/Listener.hpp"
#include "config/ServerBlock.hpp"

class	WebServer{
	private:
		std::unordered_map<std::string, std::vector<std::string>> _portHost;
		std::vector<Listener>       _theLList;

		WebServer obj(const WebServer& other) = delete;
		WebServer& operator=(const WebServer& other) = delete;
	public:
		WebServer();
		~WebServer();

		int	initialize(std::vector<ServerBlock>& serBlocks);
		void freeStuff(void);

		std::vector<Listener>& getListeners(void);
		bool doesListenerExist(std::string port);
		int resolveListener(std::string port, std::string host, ServerBlock& serBlock);
		Listener* findListenerByPort(std::string port);	
		std::string createUniqueKey(const std::string& host, const std::string& port, const std::string& serverName);
};

