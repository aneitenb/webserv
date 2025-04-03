/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventLoop.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 14:51:49 by aneitenb          #+#    #+#             */
/*   Updated: 2025/04/03 16:43:18 by mspasic          ###   ########.fr       */
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
    std::vector<int>            _socketFds;
    std::vector<epoll_event>    _events; //for resolving events?
    // int _maxEvents = MAX_EVENTS;
public:
    EventLoop(int maxEvents);
    EventLoop();
    ~EventLoop();
    int     addToEpoll (int fd, uint32_t events);
    int     modifyEpoll();
    int     delEpoll();
    void    run(); //epoll_wait + resolve events: accept/send/recv
};