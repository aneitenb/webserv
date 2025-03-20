/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aneitenb <aneitenb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 14:46:54 by aneitenb          #+#    #+#             */
/*   Updated: 2025/03/20 17:55:23 by aneitenb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../../srcs/server/Server.hpp"


/*********************************************************************
*     ATTENTION: This will change after parsing structure is changed *
**********************************************************************/
Server::Server(const std::string& configPath)
{
	try {
		_config.initialize(configPath);
		
		std::vector<ServerBlocks> serverConfigs = _config.getServers();
		std::vector<size_t> ports = _config.getPorts;
		
		//VirtualServer is calling the object constructor and passing the server configuration
		for (size_t i = 0; i < serverConfigs.size(); i++)
		{
			_virtualServer.push_back(VirtualServer(serverConfigs[i]]);
		}

		//setup event loop? with required ports?
		_eventLoop = EventLoop("what to pass?");

		//set up server sockets
		for (int i = 0; i < ports.size() < i++)
		{
			SetupServerSocket(ports[i]);
		}
		
	}
	catch (const std::exception e) {
		std::err << "Error initializing server: " << e.what() << std::endl;
		throw;
	}
}