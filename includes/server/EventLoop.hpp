/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventLoop.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 14:51:49 by aneitenb          #+#    #+#             */
/*   Updated: 2025/04/10 19:11:46 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <vector>
#include "VirtualHost.hpp"
#include <sys/epoll.h>

#define MAX_EVENTS 1024


//from sys/epoll.h
// struct epoll_event {
//     uint32_t      events;  /* Epoll events */
//     epoll_data_t  data;    /* User data variable */
//      or? 
//     void* data.ptr /*can be a ptr to a connection class*/
// };

// union epoll_data {
//     void     *ptr;
//     int       fd;
//     uint32_t  u32;
//     uint64_t  u64;
// };



// epoll/select handling
class   EventLoop{
private:
    int                         _epollFd; //for epoll_create1
    std::vector<int *>            _fds;
    std::vector<epoll_event>    _events; //for resolving events?
    // int _maxEvents = MAX_EVENTS;
public:
    EventLoop(int maxEvents);
    EventLoop();
    ~EventLoop();
    void addListenerFds(std::vector<Listener>& listFds);
    int addToEpoll (int& fd, uint32_t events);
    int modifyEpoll();
    int delEpoll();
    int startRun();
    int run(std::vector<Listener>& listFds); //epoll_wait + resolve events: accept/send/recv
};