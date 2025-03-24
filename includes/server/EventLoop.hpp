/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EventLoop.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 14:51:49 by aneitenb          #+#    #+#             */
/*   Updated: 2025/03/24 16:40:29 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <vector>

#define MAX_EVENTS 1024

// epoll/select handling
class   EventLoop{
private:
    int _epoll_fd; //for epoll_create1
    std::vector<int> socket_fds;
    // int _max_events = MAX_EVENTS;
public:
    EventLoop(int maxEvents);
    ~EventLoop();
    
}