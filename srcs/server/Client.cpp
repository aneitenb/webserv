/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/06 23:04:40 by mspasic           #+#    #+#             */
/*   Updated: 2025/04/11 22:48:30 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/Client.hpp"
#include "CommonFunctions.hpp"
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <fcntl.h>

Client::Client(EventLoop& curLoop):_listfd(nullptr), _clFd(-1){
    ftMemset(&_result, sizeof(_result));
    _loop = &curLoop;
    // ftMemset(&_event, sizeof(_event)); //do I leave this like this?
}

Client::~Client(){
    if (_clFd != -1){
        close (_clFd);
        _clFd = -1;
    }
}

void Client::copySocketFd(int* fd){
    if (_clFd != -1){
        close(_clFd);
        _clFd = -1;}
    _clFd = dup(*fd);
    if (_clFd == -1)
        std::cout << "Error: dup() failed\n"; //exit?
    close(*fd);
    *fd = -1;
}

Client::Client(Client&& other) noexcept {
    _listfd = other._listfd;
    other._listfd = nullptr;
    this->copySocketFd(&other._clFd);
    _result = other._result;
    other._result = nullptr;
    _cur = other._cur;
}

//this should never be used though
Client& Client::operator=(Client&& other) noexcept {
    if (this != &other){
        _listfd = other._listfd;
        other._listfd = nullptr;
        this->copySocketFd(&other._clFd);
        _result = other._result;
        other._result = nullptr;
        _cur = other._cur;
    }
    return (*this);
}

State Client::getState(void) const{
    return(_cur);
}

void Client::setState(State newState){
    _cur = newState;
}

int Client::settingUp(int* fd){
    socklen_t addr_size = sizeof(struct sockaddr*);
    //this should maybe be handled by a listener?
    if ((_clFd = accept(*fd, _result, &addr_size) == -1)){
        std::cerr << "Error: accept() failed; could not accept client\n";
        std::cerr << strerror(errno) << "\n";
        return (-1);
    }
    if (fcntl(_clFd, F_SETFL, O_NONBLOCK) == -1){
        std::cerr << "Error: difailed to manipulate client flags\n";
        std::cerr << strerror(errno) << "\n";
        return (-1);        
    }   
    return (0);
}

void Client::setLoop(EventLoop& curLoop){
    _loop = &curLoop;
}

EventLoop& Client::getLoop(void){
    return (*_loop);
}