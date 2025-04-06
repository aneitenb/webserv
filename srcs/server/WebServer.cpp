/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/03 16:21:54 by mspasic           #+#    #+#             */
/*   Updated: 2025/04/06 21:50:38 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"

WebServer::WebServer(){}
WebServer::~WebServer(){}


void WebServer::initialize(std::vector<ServerBlock> _ServerBlock){
    std::vector<std::string> curPorts;
    std::size_t counter;

    for (std::size_t i = 0; i < _ServerBlock.size(); i++){
        curPorts = _ServerBlock.at(i).getListen();
        counter = curPorts.size();
        for (std::size_t j = 0; j < counter; j++)
        {
            VirtualHost cur(_ServerBlock.at(i), curPorts.at(j));
            _virtualHosts.push_back(cur);
        }
        if (curPorts.size() != 1){

        }
        while(size_t i = 0; i < _ServerBlock; i++){
            
        }
        
       
    }
    //go through the info from the server blocks add socket_fd
    //save stuff in virtualhost or do we even need this, maybe we just add it to the server block?
    //check for multiple hosts and save listeners
    //make listeners nonbloccking etc
    //figure out error handling
}