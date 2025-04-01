/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aneitenb <aneitenb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/20 12:50:45 by aneitenb          #+#    #+#             */
/*   Updated: 2025/04/01 17:39:19 by aneitenb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Webserv.hpp"
#include "../includes/config/ConfigFile.hpp"
#include "../includes/config/ConfigErrors.hpp"
//#include "server/Server.hpp"

void displayServerInfo(const ConfigurationFile& config);

int main(int ac, char **av)
{
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
	
	// Parse configuration file
	ConfigurationFile config;
	try {
		config.initialize((std::string)av[1]);
		std::cout << "Configuration file parsed successfully!" << std::endl;
		displayServerInfo(config);
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

void displayServerInfo(const ConfigurationFile& config)
{
	size_t serverCount = config.getServerCount();
	std::cout << "\n-------------------------------------" << std::endl;
	std::cout << "Found " << serverCount << " server(s)" << std::endl;
	
	for (size_t i = 0; i < serverCount; i++) {
		const ServerBlock& server = config.getServerBlock(i);
		
		std::cout << "\n----- Server " << i + 1 << " -----" << std::endl;
		std::cout << "Host: " << server.getHost() << std::endl;
		std::cout << "Port: " << server.getListen() << std::endl;
		std::cout << "Server Name: " << server.getServerName() << std::endl;
		std::cout << "Root: " << server.getRoot() << std::endl;
		std::cout << "Max Body Size: " << server.getClientMaxBodySize() << " bytes" << std::endl;
		
		// Display error pages
		std::vector<std::pair<int, std::string>> errorPages = server.getErrorPages();
		if (!errorPages.empty()) {
			std::cout << "Error Pages:" << std::endl;
			for (const auto& page : errorPages) {
				std::cout << "  " << page.first << ": " << page.second << std::endl;
			}
		}
		
		// Display location blocks
		std::map<std::string, LocationBlock> locations = server.getLocationBlocks();
		if (!locations.empty()) {
			std::cout << "Location Blocks:" << std::endl;
			for (const auto& loc : locations) {
				std::cout << "  Location: " << loc.first << std::endl;
				
				// Display location details
				if (loc.second.hasRedirect()) {
					std::cout << "    Redirect: " << loc.second.getRedirect().first 
							  << " -> " << loc.second.getRedirect().second << std::endl;
				}
				
				// Access autoindex directly since there's no hasAutoindex method
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
				
				// Show allowed methods using your allowedMethodsToString method
				std::cout << "    Allowed Methods: " << loc.second.allowedMethodsToString() << std::endl;
			}
		}
	}
	std::cout << "-------------------------------------\n" << std::endl;
}
