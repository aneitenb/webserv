/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/20 12:50:45 by aneitenb          #+#    #+#             */
/*   Updated: 2025/04/07 21:58:12 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/server/WebServer.hpp"
#include "../includes/config/ServerBlock.hpp"

int main()
{
	// if (argc != 2)
	// {
	// 	std::cerr << "Error: Wrong amount of parameters. Cannot initialize server." << std::endl;
	// 	return (1);
	// }

	try
	{
		// ConfigurationFile config;
		
		// // Set configuration file path
		// if (argc == 1)
		// {
		// 	config.initializeConfFile("");
		// 	std::cout << "Using default configuration file" << std::endl;
		// }
		// else
		// {
		// 	config.initializeConfFile(argv[1]);
		// 	std::cout << "Using provided configuration file: " << argv[1] << std::endl;
		// }
		// std::cout << "Configuration initialized successfully!" << std::endl;
		
		// // Get configuration data
		// const std::vector<ServerBlock>& servers = config.getServers();
		// const std::vector<size_t>& ports = config.getPorts();

		// std::cout << "\nConfiguration Summary:" << std::endl;
		// std::cout << "Number of servers configured: " << servers.size() << std::endl;
		
		// std::cout << "Configured ports: ";
		// for (size_t i = 0; i < ports.size(); ++i)
		// {
		// 	std::cout << ports[i];
		// 	if (i < ports.size() - 1)
		// 		std::cout << ", ";
		// }
		// std::cout << std::endl;

		// // Print each server's info
		// for (size_t i = 0; i < servers.size(); ++i)
		// {
		// 	std::cout << "\nServer " << i + 1 << " Configuration:" << std::endl;
		// 	std::map<std::string, std::string>::const_iterator it;
		// 	for (it = servers[i].begin(); it != servers[i].end(); ++it)
		// 	{
		// 		std::cout << "  " << it->first << ": " << it->second << std::endl;
		// 	}
		// }
		WebServer instance;
		std::vector <ServerBlock> testingServ;
		
		// Future: Set up and run servers
		ServerBlock test;
		test.addErrorPage(1, "/example/path");
		test.addListen("8080");
		test.setClientMaxBodySize(30);
		test.setHost("127.0.0.1");
		test.setIndex("dunno");
		test.setRoot("/");
		test.setServerName("dorian");
		testingServ.push_back(test);
		ServerBlock test1;
		test.addErrorPage(2, "/example/path");
		test.addListen("8181");
		test.setClientMaxBodySize(30);
		test.setHost("127.0.0.1");
		test.setIndex("dunno");
		test.setRoot("/");
		test.setServerName("dorian");
		testingServ.push_back(test1);
		ServerBlock test2;
		test.addErrorPage(3, "/example/path");
		test.addListen("8080");
		test.setClientMaxBodySize(30);
		test.setHost("127.0.0.2");
		test.setIndex("dunno");
		test.setRoot("/");
		test.setServerName("shlorpian");
		testingServ.push_back(test2);
		std::cout << "Servers initialised\n";

		instance.initialize(&testingServ);

		return (0);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return (1);
	}
}