/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aneitenb <aneitenb@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/20 12:50:45 by aneitenb          #+#    #+#             */
/*   Updated: 2025/02/11 15:25:22 by aneitenb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Webserv.hpp"
#include "../includes/ConfigFile.hpp"

int main(int argc, char **argv)
{
	if (argc > 2)
	{
		std::cerr << "Error: Wrong amount of parameters. Cannot initialize server." << std::endl;
		return (1);
	}

	try
	{
		ConfigurationFile config;
		std::string configFile;
		
		// Set configuration file path
		if (argc == 1)
		{
			configFile = "./configuration/basic.conf";
			std::cout << "Using default configuration file: " << configFile << std::endl;
		}
		else
		{
			configFile = argv[1];
			std::cout << "Using provided configuration file: " << configFile << std::endl;
		}
		
		// Initialize configuration
		config.initializeConfFile(configFile);
		std::cout << "Configuration initialized successfully!" << std::endl;
		
		// Get configuration data
		const std::vector<ServerBlocks>& servers = config.getServers();
		const std::vector<size_t>& ports = config.getPorts();

		std::cout << "\nConfiguration Summary:" << std::endl;
		std::cout << "Number of servers configured: " << servers.size() << std::endl;
		
		std::cout << "Configured ports: ";
		for (size_t i = 0; i < ports.size(); ++i)
		{
			std::cout << ports[i];
			if (i < ports.size() - 1)
				std::cout << ", ";
		}
		std::cout << std::endl;

		// Print each server's info
		for (size_t i = 0; i < servers.size(); ++i)
		{
			std::cout << "\nServer " << i + 1 << " Configuration:" << std::endl;
			std::map<std::string, std::string>::const_iterator it;
			for (it = servers[i].begin(); it != servers[i].end(); ++it)
			{
				std::cout << "  " << it->first << ": " << it->second << std::endl;
			}
		}

		std::cout << "\nServer initialized successfully. Ready for implementation of server logic." << std::endl;
		
		// Future: Set up and run servers
		// TODO: Add server initialization and running logic

		return (0);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return (1);
	}
}