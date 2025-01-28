/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aneitenb <aneitenb@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/20 12:50:45 by aneitenb          #+#    #+#             */
/*   Updated: 2025/01/28 14:56:45 by aneitenb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Webserv.hpp"
#include "../includes/ConfigFile.hpp"



int main (int argc, char **argv)
{
	if (argc > 2)
	{
		std::cerr << "Webserv: Wrong amount of parameters. Cannot initialize server." << std::endl;
		return (1);
	}

	try
	{
		ConfigurationFile config;
		std::string configFile;
		
		if (argc == 1)  // No configuration file provided
		{
			configFile = "./configuration/basic.conf";
			std::cout << "Using default configuration file:" << configFile << std::endl;
		}
		else  // Configuration file provided as argument
		{
			configFile = argv[1];
			std::cout << "Using provided configuration file: " << configFile << std::endl;
		}
		
		config.initializeConfFile(configFile);
		std::cout << "Configuration initialized successfully!" << std::endl;
		
		/*
		* Get configuration data (for future use)
		* Lets set up a server manager class that'll use this information to create 
		* and initialize multiple server instances - one for each configuration 
		* in the servers vector. Each server will be set up to listen on its 
		* specified port (from ports), handle incoming HTTP requests according 
		* to its configuration (routing requests to correct directories, handling 
		* allowed methods, serving error pages when needed), and manage client 
		* connections
		*/
		const ServerConfigs& servers = config.getServers();
		const std::vector<size_t>& ports = config.getPorts();
		(void)servers;
		(void)ports;

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
	return (0);
}