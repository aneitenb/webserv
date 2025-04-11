/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventLoop.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 19:32:39 by mspasic           #+#    #+#             */
/*   Updated: 2025/04/11 22:49:06 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/EventLoop.hpp"
#include "EventHandler.hpp"
#include <string.h>
#include <iostream> //cerr
#include <unistd.h> //close

EventLoop::EventLoop() : _epollFd(-1){}

EventLoop::~EventLoop(){
    if (_epollFd != -1){
        (_epollFd);
        _epollFd = -1;
    }
}

int EventLoop::addListeners(std::vector<Listener>& listFds){
    for (std::size_t i = 0; i < listFds.size(); i++){
        struct epoll_event curE;
        curE.events = EPOLLIN | EPOLLONESHOT;
        curE.data.fd = *listFds.at(i).getSocketFd();
        curE.data.ptr = static_cast<void*>(&listFds.at(i));

        if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, *listFds.at(i).getSocketFd(), &curE) == -1){
            std::cerr << "Error: Could not add the file descriptor to the epoll instance\n";
            strerror(errno);
            return (-1);
        }
        listFds.at(i).setLoop(*this);
    }
    return (0);
}

int EventLoop::startRun(void){
    //set up
    if ((_epollFd = epoll_create1(0)) == -1){
        std::cerr << "Error: Could not create epoll instance\n";
        strerror(errno);
        return (-1); //won't clean up the other things:/
    }
    return (0);
}

int EventLoop::run(std::vector<Listener>& listFds){

    if (this->startRun() == -1)
        return (-1);
    if (this->addListeners(listFds) == -1)
        return (-1);
    while(1){
        int events2Resolve = epoll_wait(_epollFd, _events, MAX_EVENTS, -1);
        for (int i = 0; i < events2Resolve; i++){
            EventHandler* curE = static_cast<EventHandler*>(_events[i].data.ptr);
            if (curE->handleEvent(_events[i].events) == -1)
                return (-1); //cleanup
        }
    }   
}

