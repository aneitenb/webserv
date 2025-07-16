// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<main.cpp>> -- <<Aida, Ilmari, Milica>>

#include "utils/message.hpp"
#include "config/ConfigFile.hpp"
#include "config/ConfigErrors.hpp"
#include "server/WebServer.hpp"
#include "server/EventLoop.hpp"
#include <csignal>

/*a visible side-effect for the purposes of optimization 
(that is, within a single thread of execution, volatile accesses cannot be optimized out or reordered with 
another visible side effect that is sequenced-before or sequenced-after the volatile access. 
This makes volatile objects suitable for communication with a signal handler, but not with another thread of execution*/
volatile sig_atomic_t gSignal = 1;

void displayServerInfo(const ConfigurationFile& config);

/*extern "C": makes a function-name in C++ have C linkage (compiler does not mangle the name) 
so that client C code can link to (use) the function using a C compatible header file 
that contains just the declaration of the function*/

extern "C" void signalHandler(int signum) {
	gSignal = 0;
	Debug("\nSignal received: " << signum);
	return ;
}


int program(char** av) {
	ConfigurationFile config;
	try {
		config.initialize((std::string)av[1]);
		info("Configuration file parsed successfully");
		displayServerInfo(config);

		WebServer instance;
		EventLoop epolling;
		std::vector<EventHandler*> listPtrs;

		//use ServerBlocks to init webserver instance
		if (instance.initialize(config.getAllServerBlocks()) == -1) {
			instance.freeStuff();
			return 1; //free addresses?
		}

		//Listener -> EventHandler* so I can pass it to EventLoop
		std::vector<Listener>& listeners = instance.getListeners();
		listPtrs.reserve(listeners.size());
		for (std::size_t m = 0; m < listeners.size(); m++){
			std::cout << listeners.at(m).getFirstKey() << ": first key\n";
			std::cout << *(listeners.at(m).getSocketFd()) << ": fd\n\n";
		}
		std::cout << "Came here\n\n";
		for (auto& obj : listeners)
			listPtrs.push_back(&obj);
		epolling.run(listPtrs);
		instance.freeStuff();
	} catch (const ConfigError& e) {
		Error("Fatal: " << e.getErrorType() << ": " << e.what());
		return 1;
	} catch (const std::exception& e) {
		Error("Fatal: Unexpected error: " << e.what());
		return 1;
	}
	return 0;	
}


int main(int ac, char **av) {
	DIR	*dir;

	signal(SIGPIPE, SIG_IGN);
	std::signal(SIGINT, signalHandler);
	std::signal(SIGCHLD, signalHandler);

	if (ac != 2 || av[1] == nullptr || av[1][0] == '\0') {
		error("Fatal: Expected only configuration file as argument");
		return 1;
	}
	
	try {
		dir = opendir(av[1]);
		if (dir != NULL) {
			closedir(dir);
			throw std::invalid_argument("Argument is a directory");
		}
		if (std::filesystem::exists((std::string)av[1]) && std::filesystem::file_size((std::string)av[1]) == 0) {
			error("Fatal: Configuration file is empty");
			return 1;
		}
	} catch (std::exception& e) {
		Error("Fatal: " << e.what());
		return 1; 
	}
	
	return program(av);
}

void displayServerInfo(const ConfigurationFile& config) {
	std::stringstream	tmp;
	size_t				serverCount = config.getServerCount();

	Info("\nFound " << serverCount << " server" << ((serverCount > 1) ? 's' : '\0'));
	
	for (size_t i = 0; i < serverCount; i++) {
		const ServerBlock& server = config.getServerBlock(i);
		
		Info("\nServer " << i + 1 << ':');
		Info("  Name:            " << server.getServerName());
		tmp = std::stringstream("") << "  Address:         " << server.getHost() << ':';
		const std::vector<std::string>& ports = server.getListen();
		for (size_t i = 0; i < ports.size(); ++i) {
			tmp << ports[i];
			if (i < ports.size() - 1) {
				tmp << ':';
			}
		}
		info(tmp);
		Info("  Root:            " << server.getRoot());
		Info("  Index:           " << server.getIndex());
		Info("  Max Body Size:   " << server.getClientMaxBodySize() << 'B');
		if (server.hasAllowedMethods())
			Info("  Allowed Methods: " << server.allowedMethodsToString());

		// Display error pages
		std::vector<std::pair<int, std::string>> errorPages = server.getErrorPages();
		if (!errorPages.empty()) {
			info("\n  Special Error Pages:");
			for (const auto& page : errorPages)
				Info("    "<< page.first << ": " << page.second);
		}

		//Default error pages
		std::vector<std::pair<int, std::string>> defaultErrorPages = server.getDefaultErrorPages();
		debug("\n  Default Error Pages:");
		for (const auto& page : defaultErrorPages)
			Debug("    "<< page.first << ": " << page.second);
		
		// Display location blocks
		std::map<std::string, LocationBlock> locations = server.getLocationBlocks();
		if (!locations.empty()) {
			Info("\n  Location Blocks:");
			for (const auto& loc : locations) {
				Info("    Location: " << loc.first);
				if (loc.second.hasRedirect())
					Info("      Redirect: " << loc.second.getRedirect().first << " -> " << loc.second.getRedirect().second);
				Info("      Autoindex: " << ((loc.second.getAutoindex()) ? "on" : "off"));
				if (loc.second.hasCgiPass())
					Info("      CGI Pass: " << loc.second.getCgiPass());
				if (loc.second.hasUploadStore())
					Info("      Upload Store: " << loc.second.getUploadStore());
				if (loc.second.hasAlias())
					Info("      Alias: " << loc.second.getAlias());
				if (loc.second.hasIndex())
					Info("      Index: " << loc.second.getIndex());
				Info("      Allowed Methods: " << loc.second.allowedMethodsToString());
			}
		}
	}
	info("");
}
