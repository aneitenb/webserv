/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventLoop.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 19:32:39 by mspasic           #+#    #+#             */
/*   Updated: 2025/03/24 20:09:18 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../server/EventLoop.hpp"
#include <string.h>

EventLoop::EventLoop(int maxEvents){
    //clean up just in case
    _events.clear();
    _epollFd = -1;
    //set up
    if ((_epollFd = epoll_create1(0)) == -1){
        std::cerr << "Error: Could not create epoll instance\n";
        strerror(errno);
        exit (EXIT_FAILURE); //won't clean up the other things:/
    }
}

EventLoop::~EventLoop(){
    close (_epollFd);
}

int EventLoop::addToEpoll(int fd, uint32_t events){

}

void EventLoop::run(){
    while(1){

    }
}
