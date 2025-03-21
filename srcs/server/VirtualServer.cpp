/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualServer.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 16:26:02 by mspasic           #+#    #+#             */
/*   Updated: 2025/03/21 18:25:06 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "VirtualServer.hpp"

// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <unistd.h>
#include <fcntl.h> //fcntl
#include <netdb.h> //getaddrinfo
// #include <poll.h>
#include <cstring> //memset

#define IP "127.0.0.1"

VirtualServer::VirtualServer(){
    std::cout << "Creating client server\n";
}

VirtualServer::VirtualServer(ConfigurationFile _config){
    std::cout << "Creating listening socket\n";
    memset(&_address, 0, sizeof(_address));
    _address.sin_port = htons(PORT);
    _address.sin_family = AF_INET;
    memset(&(_address.sin_zero), '\0', 8); //zero the rest of the struct
    
    if (inet_pton(AF_INET, IP, &(_address.sin_addr.s_addr)) <= 0){
        std::cerr << "Error: socket() failed\n";
        std::cerr << strerror(errno) << "\n";
        return (-1);
    }
    _addr_size = sizeof(_address);
}

VirtualServer::~VirtualServer(){
    std::cout << "Virtual server destroyed\n";
    close(_sockfd);
}

int VirtualServer::setup_fd(void){
    if (_sockfd = socket(PF_INET, SOCK_STREAM, 0) == -1){
        std::cerr << "Error: socket() failed\n";
        std::cerr << strerror(errno) << "\n";
        return (-1);
    }
    //setsockopt: manipulate options for the socket 
    //CONSIDER: SO_RCVBUF / SO_SNDBUF, SO_LINGER, SO_KEEPALIVE, TCP_NODELAY
    //get socket error
    if ((setsockopt(_sockfd, SOL_SOCKET, SO_ERROR, &_sock_err, sizeof(_sock_err))) == -1){
        std::cerr << "Error: sersockopt() failed: SO_ERROR\n";
        std::cerr << strerror(errno) << "\n";
        return (-1);
    }
    //reuse port, multiple bind on the same port
    int reuse_port = 1;
    if ((setsockopt(_sockfd, SOL_SOCKET, SO_REUSEPORT, &reuse_port, sizeof(reuse_port))) == -1){
        std::cerr << "Error: sersockopt() failed: SO_REUSEPORT\n";
        std::cerr << strerror(errno) << "\n";
        return (-1);
    }
    //reuse address: reuse a local address or port that is in the TIME_WAIT state (e.g., after closing a socket
    int reuse_addr = 1;
    if ((setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr))) == -1){
        std::cerr << "Error: sersockopt() failed: SO_REUSEADDR\n";
        std::cerr << strerror(errno) << "\n";
        return (-1);
    }
    //make it non-blocking
    if ((fcntl(_sockfd, F_SETFL, O_NONBLOCK)) == -1){
        std::cerr << "Error: fcntl() failed\n";
        std::cerr << strerror(errno) << "\n";
        return (-1);
    }
}