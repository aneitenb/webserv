/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventLoop.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 19:32:39 by mspasic           #+#    #+#             */
/*   Updated: 2025/04/10 19:23:23 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/EventLoop.hpp"
#include <string.h>
#include <iostream> //cerr
#include <unistd.h> //close

EventLoop::EventLoop(){};

EventLoop::EventLoop(int maxEvents){
    //clean up just in case
    _events.clear();
    _epollFd = -1;

}

EventLoop::~EventLoop(){
    close (_epollFd);
}

void EventLoop::addListenerFds(std::vector<Listener>& listFds){
    for (std::size_t i = 0; i < listFds.size(); i++){
        _fds.emplace_back(listFds.at(i).getSocketFd());
    }
}


// int EventLoop::addToEpoll(int& fd, uint32_t events){

// }

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

    this->addListenerFds(listFds);
    this->startRun();
    while(1){
        //wait
        //check if wait failed
            //if errno == EINTR continue look this up
            //error handling
        //process events; wait returns the number of events that need to be resolved
            //if there is something to be resolved with one of the listening sockets
            //means a new connection can be accepted
                //accept, check the clientfd, set to nonblock, addto epoll with ein and elet
                //new Connection class object
            //otherwise
                //look through the connections for the correct fd to resolve
                    //check for errors or disconnect //epollerr | epollhup
                    //remove from epoll
                    //delete connection object
                    //vector.erase(currentfd)
                    //close(currentfd)
                    //continue
                //if its epollin and not a listening socket, means that it can be read
                    //figure it out
                //if its epollout and not a listening socket, means that it can be written into
                    //figure it out
//*needs a map/dictionary to connect fds and connections?
    }   
}
