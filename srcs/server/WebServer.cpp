/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/03 16:21:54 by mspasic           #+#    #+#             */
/*   Updated: 2025/04/04 18:27:05 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"

WebServer::WebServer(){}
WebServer::~WebServer(){}






void WebServer::initialize(std::vector<ServerBlocks> _serverBlocks){
    for (int i = 0; i < _serverBlocks.size(); i++){
//cant access with i
        
       
    }
    //go through the info from the server blocks add socket_fd
    //save stuff in virtualhost or do we even need this, maybe we just add it to the server block?
    //check for multiple hosts and save listeners
    //make listeners nonbloccking etc
    //figure out error handling
}