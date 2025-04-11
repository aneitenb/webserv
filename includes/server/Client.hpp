/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/03 15:36:48 by mspasic           #+#    #+#             */
/*   Updated: 2025/04/11 22:48:05 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <sys/epoll.h>
#include "EventHandler.hpp"
#include "EventLoop.hpp"

class Client : public EventHandler {
    private:
        int*                _listfd;
        int                 _clFd;
        struct sockaddr*    _result;
        // struct epoll_event  _event;
        State _cur;
        EventLoop* _loop;
    public:
        Client();
        Client(EventLoop& curLoop);
        ~Client();
        Client(const Client& other) = delete;
        Client& operator=(const Client& other) = delete;        // int     getFlag(void) const;
        //move constructor
        Client(Client&& other) noexcept;
        Client& operator=(Client&& other) noexcept;
        // void    setFlag(int newState);
        int     settingUp(int* fd);
        State getState() const;
        void setState(State newState);
        void copySocketFd(int* fd);
        void setLoop(EventLoop& curLoop);
        EventLoop& getLoop(void);
        //eventhandler
};

enum State {
    WRITING,
    READING,
    CLOSE
};