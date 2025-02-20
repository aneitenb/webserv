/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/20 12:47:42 by aneitenb          #+#    #+#             */
/*   Updated: 2025/02/20 17:49:47 by mspasic          ###   ########.fr       */
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

/*

struct sockaddr_in {
    short            sin_family;   // e.g. AF_INET
    unsigned short   sin_port;     // e.g. htons(3490)
    struct in_addr   sin_addr;     // see struct in_addr, below
    char             sin_zero[8];  // zero this if you want to
};

struct in_addr {
    unsigned long s_addr;  // load with inet_aton()
};
*/


struct polling{
    sockaddr_in address;
    bool   listens;
    int state;
    // socklen_t   addr_size;
    struct pollfd pfd;
    char *buffer;
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
