/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Listener.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/04 15:10:44 by mspasic           #+#    #+#             */
/*   Updated: 2025/04/10 22:09:00 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/Listener.hpp"
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

Listener::Listener() : _sockFd(-1){};

Listener::Listener(std::string port, std::string host) : _sockFd(-1), _port(port), _host(host) {}

// Listener::Listener() : _sockFd(-1){};

Listener::Listener(const Listener& obj){
    _sockFd = -1;
    this->copySocketFd(obj._sockFd);
    // _port = obj._port;
    // _host = obj._host;
}

Listener& Listener::operator=(const Listener& obj) {
    if (this != &obj){
        this->copySocketFd(obj._sockFd);
        // _port = obj._port;
        // _host = obj._host;
    }
    return (*this);
}

Listener::~Listener(){
    if (_sockFd != -1){
        close(_sockFd);
        _sockFd = -1;
        // std::cout << "closed fd\n";
    }
}

int* Listener::getSocketFd(void){
    return(&_sockFd);
}

/*setting up the listening (and client?) socket to nonblocking*/
int setuping(int *fd){
    // int sock_err = 0;
    //setsockopt: manipulate options for the socket 
    //CONSIDER: SO_RCVBUF / SO_SNDBUF, SO_LINGER, SO_KEEPALIVE, TCP_NODELAY
    //get socket error
    // if ((setsockopt(*fd, SOL_SOCKET, SO_ERROR, &sock_err, sizeof(sock_err))) == -1){
    //     std::cerr << "Error: setsockopt() failed: SO_ERROR: " << sock_err << "\n";
    //     std::cerr << strerror(errno) << "\n";
    //     return (-1);
    // }
    //make it non-blocking
    if ((fcntl(*fd, F_SETFL, O_NONBLOCK)) == -1){
        std::cerr << "Error: fcntl() failed\n";
        std::cerr << strerror(errno) << "\n";
        return (-1);
    }
    std::cout << "socketFD " << *fd << " has been successfully set up as non-blocking\n";     
    return (0);
}

int Listener::setSocketFd(void){
    if (_sockFd != -1){
        close(_sockFd);
        _sockFd = -1;}
    if ((_sockFd = socket(PF_INET, SOCK_STREAM, 0)) == -1){
        std::cerr << "Error: socket() failed\n";
        std::cerr << strerror(errno) << "\n";
        return (-1);
    }
    if ((setuping(&_sockFd)) == -1)
        return (-1);
    return (0);
}

int Listener::copySocketFd(const int& fd){
        if (_sockFd != -1){
            close(_sockFd);
            _sockFd = -1;}
        _sockFd = dup(fd);
        if (_sockFd == -1)
            std::cout << "Error: dup() failed\n";
        // close(fd);
        return (_sockFd);
}

std::vector<VirtualHost> Listener::getHosts(void) const{
    return (_knownVHs);
}

void Listener::addHost(VirtualHost& cur){
    _knownVHs.push_back(cur);
}

const std::string& Listener::getPort(void) const{
    return (_port);
}


void Listener::setPort(const std::string& port){
    _port = port;
}

        
const std::string& Listener::getHost(void) const{
    return (_host);
}

void Listener::setHost(const std::string& host){
    _host = host;
}

void Listener::acceptClient(void){
    //code
}