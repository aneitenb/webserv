/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/06 23:04:40 by mspasic           #+#    #+#             */
/*   Updated: 2025/04/06 23:49:32 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include "CommonFunctions.hpp"
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <fcntl.h>

Client::Client():_listfd(nullptr), _clFd(-1), _state(-1){
    ftMemset(&_result, sizeof(_result));
    ftMemset(&_event, sizeof(_event)); //do I leave this like this?
};

Client::~Client(){
    if (_clFd != -1){
        close (_clFd);
        _clFd = -1;
    }
    if (_state != -1)
        _state = -1;
}

int Client::getFlag(void) const{
    return(_state);
}

void Client::setFlag(int newState){
    _state = newState;
}

int Client::settingUp(int* fd){
    socklen_t addr_size = sizeof(struct sockaddr*);
    //do i need to know the client's address and etc.? for security reasons, sure
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
