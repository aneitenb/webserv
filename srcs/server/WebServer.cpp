/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/03 16:21:54 by mspasic           #+#    #+#             */
/*   Updated: 2025/04/03 16:53:40 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"

WebServer::WebServer(){}
WebServer::~WebServer(){}


void WebServer::initialize(std::vector<ServerBlocks> _serverBlocks){
    //go through the info from the server blocks
    //save stuff in virtualhost
    //check for multiple hosts and save listeners
    //make listeners nonbloccking etc
    //figure out error handling
}