/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualHost.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 16:26:51 by mspasic           #+#    #+#             */
/*   Updated: 2025/04/06 23:18:58 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

// #include "config"
#include <arpa/inet.h> //uints, sockaddr_in
#include <string> //std::string
#include <sys/epoll.h> //struct epoll_event
#include <ConfigFile.hpp> //to become serverblock

#define PORT 8080
#define IP "127.0.0.1"

//socket type
#define LISTENING 0
#define CLIENT 1

class VirtualHost {
    private:
        ServerBlock        _info;
        struct addrinfo*    _result; //needs to be freed freeaddrinfo
        const char*         _port; /*or is all this going to stay parsend in the conif class and we just point at it here?*/
        const char*         _IP;
        const char*         _serv_name;
        int                 *_sockfd;
        // struct sockaddr_in  _address;
        // socklen_t           _addr_size;
        int                 _sock_err; //not needed?
        // int                 _type;
        struct epoll_event  _event;
        //locations oor a config file?
        VirtualHost() = default;
    public:
        VirtualHost(const ServerBlock &info, std::string port); 
        //move constructor
        VirtualHost(VirtualHost&& other) noexcept;
        VirtualHost(); // for listening sockets
        VirtualHost(int list_sock_fd); //for clients
        ~VirtualHost();
        int setup_fd(void);
        // int get_type() const;
        struct sockaddr* getAddress() const;
        socklen_t getAddressLength() const;
        int addressInfo(void);
};

// struct addrinfo {
//     int              ai_flags;
//     int              ai_family;
//     int              ai_socktype;
//     int              ai_protocol;
//     socklen_t        ai_addrlen;
//     struct sockaddr *ai_addr;
//     char            *ai_canonname;
//     struct addrinfo *ai_next;
// };