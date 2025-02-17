/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/20 12:47:42 by aneitenb          #+#    #+#             */
/*   Updated: 2025/02/17 18:25:14 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

//STREAM
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

//OTHER
#include <vector>
#include <map>
#include <algorithm>

//SOCKET
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>

//MACROS
#define TRUE 1
#define FALSE 0
//states
#define DONE 0

struct polling{
    struct pollfd pfd;
    bool   listens;
    char *buffer[1024];
    sockaddr_in address;
    socklen_t   addr_size;
    int state;
};

// class Webserv {
//     private:
//         u_int32_t   *_ports;
//         //int         _domain; in case we need to handle IP6
//         //std::string _IPs; in case we need to handle only specific IPs and not listen for all
//         std::vector <struct pollfd> *active_fds;
//         struct 
//     public:
//         Webserv();
//         ~Webserv();

// };
