/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Listener.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/04 15:10:44 by mspasic           #+#    #+#             */
/*   Updated: 2025/04/08 20:54:12 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/Listener.hpp"
#include <unistd.h>

Listener::Listener(std::string port, std::string host) : _sockFd(-1), _port(port), _host(host) {}

Listener::Listener(const Listener& obj){
    this->setSocketFd(obj._sockFd);
    _port = obj._port;
    _host = obj._host;
}

Listener& Listener::operator=(const Listener& obj) {
    if (this != &obj){
        this->setSocketFd(obj._sockFd);
        _port = obj._port;
        _host = obj._host;
    }
    return (*this);
}

Listener::~Listener(){
    if (_sockFd != -1){
        close(_sockFd);
        _sockFd = -1;
    }
}

int* Listener::getSocketFd(void){
    return(&_sockFd);
}

int Listener::setSocketFd(const int& fd){
    if (_sockFd != -1){
        close(_sockFd);
        _sockFd = -1;}
    _sockFd = dup(fd);
    if (_sockFd == -1)
        std::cout << "Error: dup() failed\n";
    // close(fd);
    return (_sockFd);
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