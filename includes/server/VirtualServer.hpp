/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualServer.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 16:26:51 by mspasic           #+#    #+#             */
/*   Updated: 2025/03/20 18:02:53 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "ConfigFile.hpp"
#include <arpa/inet.h> //uints, sockaddr_in
#include <string> //std::string

#define PORT 8080

class VirtualServer {
    private:
        uint16_t            _port; /*or is all this going to stay parsend in the conif class and we just point at it here?*/
        uint32_t            _IP;
        std::string         _ser_name;
        int                 _sockfd;
        struct sockaddr_in  _address;
        socklen_t           _addr_size;
    public:
        VirtualServer(); //default one for cients?
        VirtualServer(ConfigurationFile _config); //custom for listening sockets?
        ~VirtualServer();
};

