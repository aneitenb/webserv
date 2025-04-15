/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/06 23:04:40 by mspasic           #+#    #+#             */
/*   Updated: 2025/04/15 16:59:55 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/Client.hpp"
#include "CommonFunctions.hpp"
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <fcntl.h>

Client::Client():_listfd(nullptr), _clFd(-1), _cur(READING){
    ftMemset(&_result, sizeof(_result));
    // ftMemset(&_event, sizeof(_event)); //do I leave this like this?
}

Client::~Client(){
    if (_clFd != -1){
        close (_clFd);
        _clFd = -1;
    }
}

int Client::copySocketFd(int* fd){
    if (_clFd != -1){
        close(_clFd);
        _clFd = -1;}
    _clFd = dup(*fd);
    if (_clFd == -1){
        std::cout << "Error: dup() failed\n"; //exit?
        return (-1);
    }
    close(*fd);
    *fd = -1;
    return (0);
}

int* Client::getClFd(void){
    return (&_clFd);
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

bool Client::operator==(const Client& other){
    if (_listfd == other._listfd && _clFd == other._clFd \
    && _result == other._result && _cur == other._cur)
        return (true);
    return (false);
}

State Client::getState(void) const{
    return(_cur);
}

void Client::setState(State newState){
    _cur = newState;
}

int Client::handleEvent(uint32_t ev){
    if (ev & EPOLLERR || ev & EPOLLHUP){
        //error and clear?
    }
    if (ev & EPOLLIN){
        //data hads to be received, check if the whole thing was received
        //if the whole thing was received, change what the epoll listens for to epollout
    }
    if (ev & EPOLLOUT){
        //data to be sent
        //if the whole thing was sent change what the epoll listens for to epollin 
    }
}


bool Client::sending_stuff(){
    ssize_t len = send(_clFd, &_buffer, _buffer.size(), 0); //buffer + bytesSentSoFar, sizeof remaining bytes, 0
    if (len == -1){
        std::cerr << "Could not send data over the connected socket with the fd of " << _clFd << "\n";
        std::cerr << strerror(errno) << "\n";
        return (false);
    }
    return (true);
}


bool Client::receiving_stuff(){
    ssize_t len = 0;
    std::string temp_buff;
    _buffer.clear();
    temp_buff.clear(); //maybe I don't need this?

    while(1){
        len = recv(_clFd, &temp_buff, sizeof(temp_buff), 0); //sizeof(buffer) - 1?
        if (len == -1){ //either means that there is no more data to read or error
            if (errno == EAGAIN || errno == EWOULDBLOCK) //done reading
                break;
            std::cerr << "Could not receive data over the connected socket with the fd of " << _clFd << "\n";
            std::cerr << strerror(errno) << "\n";
            return (false); //actual error occurred
        }
        else if(len == 0) //means the client closed connection
        {
            // if ()
            //cleanup? probably fd needs to be closed?
            return (false);
        }
        else{ // means something was returned
            if (temp_buff.size() <= _buffer.max_size() - _buffer.size())
                _buffer.append(temp_buff); //append temp to buffer
            temp_buff.clear();
        }

    }
        //check if request is complete  
        //if yes, return true
        //if no, return false
}

// int Client::settingUp(int* fd){
//     socklen_t addr_size = sizeof(struct sockaddr*);
//     //this should maybe be handled by a listener?
//     if ((_clFd = accept(*fd, _result, &addr_size) == -1)){
//         std::cerr << "Error: accept() failed; could not accept client\n";
//         std::cerr << strerror(errno) << "\n";
//         return (-1);
//     }
//     if (fcntl(_clFd, F_SETFL, O_NONBLOCK) == -1){
//         std::cerr << "Error: difailed to manipulate client flags\n";
//         std::cerr << strerror(errno) << "\n";
//         return (-1);        
//     }   
//     return (0);
// }




//SIGNAL HANDLING???
// static void sigint_handler(int signo)
// {
//   (void)close(tcp_server_fd);
//   (void)close(tcp_client_fd);
//   sleep(2);
//   printf("Caught sigINT!\n");
//   exit(EXIT_SUCCESS);
// }

// void register_signal_handler(
// int signum,
// void (*handler)(int))
// {
//   if (signal(signum, handler) == SIG_ERR) {
//      printf("Cannot handle signal\n");
//      exit(EXIT_FAILURE);
//   }
// }