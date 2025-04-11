/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventLoop.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 14:51:49 by aneitenb          #+#    #+#             */
/*   Updated: 2025/04/11 22:32:46 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <vector>
#include "VirtualHost.hpp"
#include <sys/epoll.h>
#include "Listener.hpp"

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
    // std::vector<int *>            _fds;
    struct epoll_event    _events[MAX_EVENTS]; //result array for epoll_wait()
    // int _maxEvents = MAX_EVENTS;
    EventLoop(const EventLoop& other) = delete;
    const EventLoop& operator=(const EventLoop& other) = delete;
public:
    EventLoop();
    ~EventLoop();
    // void addListenerFds(std::vector<Listener>& listFds);
    int addToEpoll (int& fd, uint32_t event);
    int modifyEpoll(int& fd, uint32_t event);
    int delEpoll(int& fd);
    int startRun();
    int addListeners(std::vector<Listener>& listFds);
    int run(std::vector<Listener>& listFds); //epoll_wait + resolve events: accept/send/recv
};

/*epoll only cares about
- which fd is being monitored
- what kind of event is happening (EPOLLIN, EPOLLOUT...)
- who to notify of the event (handlers)

send only writes 1024 bytes at a time*/