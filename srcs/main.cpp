/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/20 12:50:45 by aneitenb          #+#    #+#             */
/*   Updated: 2025/02/20 18:40:50 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Webserv.hpp"
#include "../includes/ConfigFile.hpp"
#include <vector>


int	setup_socket(void){
	int	fd;
	fd = (AF_INET, SOCK_STREAM, 0);
	return (fd);
}

void	start_server(const std::vector<size_t> *ports){
	int give_size = ports->size(); //if 5 ports, size 5
	struct polling listen_soc[give_size] = {0}; //or use new?
	std::vector<pollfd> pfds;
	int result;
	
	//first set up the listening sockets
	for (size_t i = 0; i < ports->size(); i++){
		listen_soc[i].address.sin_port = htons(ports[i]);
		listen_soc[i].address.sin_family = AF_INET;
		listen_soc[i].address.sin_addr.s_addr = INADDR_ANY;
		listen_soc[i].listens = TRUE;
		listen_soc[i].state = 1;
		listen_soc[i].addr_size = sizeof(listen_soc[i].address);

		listen_soc[i].fds = setup_socket();
		if (listen_soc[i].fds == -1){
			exit(1); //add cleanup and error handling
		}
		if (fcntl(listen_soc[i].fds, F_SETFL, O_NONBLOCK) == -1){
			exit(1); //add cleanup and error handling
		}
		if (bind(listen_soc[i].fds,(struct sockaddr*)&listen_soc[i].address, listen_soc[i].addr_size)  == -1){
			exit(1); //add cleanup and error handling
		}
		if (listen(listen_soc[i].fds, 5) == -1){ //change to max number of clients i think
			exit(1); //add cleanup and error handling
		}
		pfds.push_back({listen_soc[i].fds, POLLIN, 0});
	}
	while (1){
		result = poll(pfds.data(), pfds.size(), 5000);
		switch (result){
			case -1: {
				//error handling, cleanup
				exit(1);
			}
			case 0:{
				//do we exit with a message?
				//should the server be running in this case?
				//timeout expired btw
				break ;
			}
			default:{

				break;
			}
		}
/* 
`poll() waits for events instead of constantly checking sockets.
✅ POLLIN (Readable) → Incoming data (or a new client on the listening socket).
✅ POLLOUT (Writable) → Ready to send data.
✅ POLLERR, POLLHUP, POLLNVAL → Handle errors/disconnects.
✅ Timeout Handling (poll() returning 0) → Prevent infinite blocking if no activity.
*/
		//poll(...)
		//interpret result:
			// if this is a new connection on a listening socket 
				//accept()
				//set to non-block
				//add to pollfd array
			//if client ready
				//read request
				//(continue) write (in chunks if buffer full)
			//if client closed or error occurs
				//remove from array
			//else
				//cleanup, close
				//break
	}
}

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
		const std::vector<size_t>& ports = config.getPorts(); //ports need to be in uint32t
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
		
		start_server(&ports);

		return (0);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return (1);
	}
	return (0);
}