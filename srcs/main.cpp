// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<main.cpp>> -- <<Aida, Ilmari, Milica>>

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

extern "C" void signalHandler(int signum){
	gSignal = 0;
	std::cout << signum << std::endl; //delete
	return;
}


int program(char** av){
	// Parse configuration file
	ConfigurationFile config;
	try {
		config.initialize((std::string)av[1]);
		std::cout << "Configuration file parsed successfully!" << std::endl;
		displayServerInfo(config);

		WebServer instance;
		EventLoop epolling;
		std::vector<EventHandler*> listPtrs;

		//use ServerBlocks to init webserver instance
		if (instance.initialize(config.getAllServerBlocks()) == -1)
			return 1;

		//Listener -> EventHandler* so I can pass it to EventLoop
		std::vector<Listener> listeners = instance.getListeners();
		listPtrs.reserve(listeners.size());
		for (Listener& listener : listeners)
			listPtrs.push_back(&listener);
		epolling.run(listPtrs);
		instance.freeStuff();
	}
	catch (const ConfigError& e) {
		std::cerr << e.getErrorType() << ": " << e.what() << std::endl;
		return 1;
	}
	catch (const std::exception& e) {
		std::cerr << "Unexpected error: " << e.what() << std::endl;
		return 1;
	}
	return 0;	
}


int main(int ac, char **av)
{
	std::signal(SIGINT, signalHandler);
	// std::signal(SIGPIPE, signalHandler);
	std::signal(SIGCHLD, signalHandler);
	signal(SIGPIPE, SIG_IGN);  // Ignore SIGPIPE globally so if pipe fails program doesn't crash

	if (ac != 2 || av[1] == nullptr || av[1][0] == '\0')
	{
		std::cerr << "Error: expecting only configuration file as argument" << std::endl;
		return 1;
	}
	
	try {
		if (opendir(av[1]) != NULL) {
			throw std::invalid_argument("argument is a directory");
		}
		if (std::filesystem::exists((std::string)av[1]) && std::filesystem::file_size((std::string)av[1]) == 0) 
		{
			std::cerr << "Error: config file is empty" << std::endl;
			return 1;
		}
	} 
	catch (std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return 1; 
	}
	
	program(av);
	
	return 0;
}

void displayServerInfo(const ConfigurationFile& config)
{
	size_t serverCount = config.getServerCount();
	std::cout << "\n-------------------------------------" << std::endl;
	std::cout << "Found " << serverCount << " server(s)" << std::endl;
	
	for (size_t i = 0; i < serverCount; i++) {
		const ServerBlock& server = config.getServerBlock(i);
		
		std::cout << "\n----- Server " << i + 1 << " -----" << std::endl;
		std::cout << "Host: " << server.getHost() << std::endl;
		std::cout << "Ports: ";
		const std::vector<std::string>& ports = server.getListen();
		for (size_t i = 0; i < ports.size(); ++i) {
   		 std::cout << ports[i];
			if (i < ports.size() - 1) {
	 		   std::cout << ", ";
			}
		}
		std::cout << std::endl;
		std::cout << "Server Name: " << server.getServerName() << std::endl;
		std::cout << "Root: " << server.getRoot() << std::endl;
		std::cout << "Max Body Size: " << server.getClientMaxBodySize() << " bytes" << std::endl;
		std::cout << "Index: " << server.getIndex() << std::endl;
		if (server.hasAllowedMethods()) {
			std::cout << "Allowed Methods: " << server.allowedMethodsToString() << std::endl;
		}

		std::cout << std::endl;
		// Display error pages
		std::vector<std::pair<int, std::string>> errorPages = server.getErrorPages();
		if (!errorPages.empty()) {
			std::cout << "Error Pages:" << std::endl;
			for (const auto& page : errorPages) {
				std::cout << "  " << page.first << ": " << page.second << std::endl;
			}
		}
		//Default error pages
		std::vector<std::pair<int, std::string>> defaultErrorPages = server.getDefaultErrorPages();
		std::cout << "Default Error Pages:" << std::endl;
			for (const auto& page : defaultErrorPages) {
				std::cout << "  " << page.first << ": " << page.second << std::endl;
			}
		std::cout << std::endl;
		
		// Display location blocks
		std::map<std::string, LocationBlock> locations = server.getLocationBlocks();
		if (!locations.empty()) {
			std::cout << "Location Blocks:" << std::endl;
			for (const auto& loc : locations) {
				std::cout << "  Location: " << loc.first << std::endl;
				
				if (loc.second.hasRedirect()) {
					std::cout << "    Redirect: " << loc.second.getRedirect().first 
							  << " -> " << loc.second.getRedirect().second << std::endl;
				}
				
				std::cout << "    Autoindex: " << (loc.second.getAutoindex() ? "on" : "off") << std::endl;
				
				if (loc.second.hasCgiPass()) {
					std::cout << "    CGI Pass: " << loc.second.getCgiPass() << std::endl;
				}

				if (loc.second.hasUploadStore()) {
					std::cout << "    Upload Store: " << loc.second.getUploadStore() << std::endl;
				}
				
				if (loc.second.hasAlias()) {
					std::cout << "    Alias: " << loc.second.getAlias() << std::endl;
				}

				if (loc.second.hasIndex()) {
					std::cout << "    Index: " << loc.second.getIndex() << std::endl;
				}
				
				std::cout << "    Allowed Methods: " << loc.second.allowedMethodsToString() << std::endl;
			}
		}
	}
	std::cout << "-------------------------------------\n" << std::endl;
}
