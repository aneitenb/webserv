/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualServer.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 16:26:02 by mspasic           #+#    #+#             */
/*   Updated: 2025/03/24 20:38:43 by mspasic          ###   ########.fr       */
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


VirtualServer::VirtualServer(int list_sock_fd){
    std::cout << "Creating client server//New connection from a client accepted\n";
    _type = CLIENT;
    _sockfd = -1;
    _addr_size = sizeof(_address);
    if ((_sockfd = accept(list_sock_fd, (struct sockaddr *)&_address, &_addr_size)) == -1){
        std::cerr << "Error: accept() failed; could not accept client\n";
        std::cerr << strerror(errno) << "\n";
        this->~VirtualServer();  //can i do this? 
    }

    if (fcntl(_sockfd, F_SETFL, O_NONBLOCK) == -1){
        std::cerr << "Error: difailed to manipulate client flags\n";
        std::cerr << strerror(errno) << "\n";
        this->~VirtualServer();          
    }
    //get info if you want but not needed 
    _event.data.fd = _sockfd;
    _event.events = EPOLLIN | EPOLLET;
    //add to epoll array
    //create connection object with client information
}

VirtualServer::VirtualServer(){ //arg is going to change
    std::cout << "Creating listening socket\n";
    _type = LISTENING;
    _sockfd = -1;
    memset(&_address, 0, sizeof(_address)); //clear out just in case; do we need the 2nd memset then?
    _address.sin_port = htons(PORT);
    _address.sin_family = AF_INET;
    memset(&(_address.sin_zero), '\0', 8); //zero the rest of the struct
    _addr_size = sizeof(_address);

    if (this->setup_fd() == -1)
        this->~VirtualServer(); //can I do this?
    
    _event.data.fd = _sockfd;
    _event.events = EPOLLIN | EPOLLET;
    
    //add to epoll array
}

VirtualServer::~VirtualServer(){
    std::cout << "Virtual server destroyed\n";
    if (_sockfd)
        close(_sockfd);
    _sockfd = 0;
}

int VirtualServer::setup_fd(void){
    //get the IP address
    if (inet_pton(AF_INET, IP, &(_address.sin_addr.s_addr)) <= 0){
        std::cerr << "Error: socket() failed\n";
        std::cerr << strerror(errno) << "\n";
        return (-1);
    }
    //socket()
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
    //for the listening socket bind and listen
    if ((bind(_sockfd, (struct sockaddr *)&_address, sizeof(struct sockaddr)) == -1)){
        std::cerr << "Error: bind() failed\n";
        std::cerr << strerror(errno) << "\n";
        return (-1);        
    }
    if ((listen(_sockfd, 20) == -1)){
        std::cerr << "Error: listen() failed\n";
        std::cerr << strerror(errno) << "\n";
        return (-1);        
    }
    return (0);
}

int VirtualServer::get_type(void){
    return(_type);
}
