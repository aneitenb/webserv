/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualHost.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 16:26:02 by mspasic           #+#    #+#             */
/*   Updated: 2025/03/25 17:20:09 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "VirtualHost.hpp"

// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <unistd.h> //close
#include <fcntl.h> //fcntl
#include <netdb.h> //getaddrinfo
// #include <poll.h>
#include <cstring> //memset
#include <iostream>

int VirtualHost::getAddrInfo(void){
    struct addrinfo hints;
    int status;

    hints.ai_family = AF_INET; //IPv4
    hints.ai_socktype = SOCK_STREAM; //TCP
    hints.ai_flags = AI_PASSIVE; //for binding (listening) maybe not needed if we always provide an IP or hostname
    if ((status = (getaddrinfo(_IP, _port, &hints, &_result))) != 0){
        std::cerr << "Error: getaddrinfo() failed: ";
        if (status == EAI_SYSTEM)
            std::cerr << strerror(errno) << "\n";
        else
            std::cerr << gai_strerror(status) << "\n";
        return (-1);
    }
    if (_result->ai_family != AF_INET){
        std::cerr << "Error: getaddrinfo failed but unclear why.\n";
        return (-1);
    }
}

void VirtualHost::ftMemset(void *dest, std::size_t count){
    unsigned char *p = (unsigned char*)dest;
    for (std::size_t i = 0; i < count; i++){
        p[i] = 0;
    }
}
//check if you have copy constructors everywhere since you use vectors; push_back() copies/moves objects
VirtualHost::VirtualHost(const ServerBlock &info, std::string port){
    _sockfd = nullptr;
    _sock_err = 0; //do I leave it like this?
    _info = info;
    _port = port.c_str();
    _IP = (info.getHost()).c_str();
    ftMemset(&_result, sizeof(_result));
    ftMemset(&_event, sizeof(_event)); //do I leave this like this?
}

VirtualHost::VirtualHost(VirtualHost&& other) noexcept {
    _info = other._info;
    _result = std::move(other._result);
    _port = std::move(other._port);
    _IP = std::move(other._IP);
    _serv_name = std::move(other._serv_name);
    _sockfd = std::move(other._sockfd);
    _sock_err = std::move(other._sock_err);
    _event = std::move(other._event);
}

VirtualHost::~VirtualHost(){
    if (_result)
        freeaddrinfo(_result);
    if (*_sockfd != -1){
        close(*_sockfd);
        *_sockfd = -1;
    }
}


/*after this for listening sockets and clients*/

VirtualHost::VirtualHost(int list_sock_fd){
    std::cout << "Creating client server//New connection from a client accepted\n";
    _type = CLIENT;
    _sockfd = -1;
    _addr_size = sizeof(_address);
    if ((_sockfd = accept(list_sock_fd, (struct sockaddr *)&_address, &_addr_size)) == -1){
        std::cerr << "Error: accept() failed; could not accept client\n";
        std::cerr << strerror(errno) << "\n";
        this->~VirtualHost();  //can i do this? 
    }

    if (fcntl(_sockfd, F_SETFL, O_NONBLOCK) == -1){
        std::cerr << "Error: difailed to manipulate client flags\n";
        std::cerr << strerror(errno) << "\n";
        this->~VirtualHost();          
    }
    //get info if you want but not needed 
    _event.data.fd = _sockfd;
    _event.events = EPOLLIN | EPOLLET;
    //add to epoll array
    //create connection object with client information
}

VirtualHost::VirtualHost(){ //arg is going to change
    // std::cout << "Creating listening socket\n";
    // _type = LISTENING;
    // _sockfd = -1;
    // memset(&_address, 0, sizeof(_address)); //clear out just in case; do we need the 2nd memset then?
    // _address.sin_port = htons(PORT);
    // _address.sin_family = AF_INET;
    // memset(&(_address.sin_zero), '\0', 8); //zero the rest of the struct
    // _addr_size = sizeof(_address);
    //get the IP address; IP is std::string   NOT ALLOWED?

    
    // if (inet_pton(AF_INET, IP, &(_address.sin_addr.s_addr)) <= 0){
    //     std::cerr << "Error: inet_pton() failed\n";
    //     std::cerr << strerror(errno) << "\n";
    //     return (-1);
    // }

    if (this->setup_fd() == -1)
        this->~VirtualHost(); //can I do this?
    
    _event.data.fd = _sockfd;
    _event.events = EPOLLIN | EPOLLET;
    
    //add to epoll array
}

VirtualHost::~VirtualHost(){
    std::cout << "Virtual server destroyed\n";

}

int VirtualHost::setup_fd(void){

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
        std::cerr << "Error: setsockopt() failed: SO_ERROR\n";
        std::cerr << strerror(errno) << "\n";
        return (-1);
    }
    //reuse port, multiple bind on the same port
    int reuse_port = 1;
    if ((setsockopt(_sockfd, SOL_SOCKET, SO_REUSEPORT, &reuse_port, sizeof(reuse_port))) == -1){
        std::cerr << "Error: setsockopt() failed: SO_REUSEPORT\n";
        std::cerr << strerror(errno) << "\n";
        return (-1);
    }
    //reuse address: reuse a local address or port that is in the TIME_WAIT state (e.g., after closing a socket
    int reuse_addr = 1;
    if ((setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr))) == -1){
        std::cerr << "Error: setsockopt() failed: SO_REUSEADDR\n";
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

int VirtualHost::get_type(void){
    return(_type);
}
