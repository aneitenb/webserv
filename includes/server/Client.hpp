/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/03 15:36:48 by mspasic           #+#    #+#             */
/*   Updated: 2025/04/12 00:23:15 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <sys/epoll.h>
#include "EventHandler.hpp"
#include "EventLoop.hpp"

class Client : public EventHandler {
    private:
        int*                _listfd; //do i need this
        int                 _clFd;
        struct sockaddr*    _result; //do i need this if when i accept i just take the fd?
        // struct epoll_event  _event;
        State _cur;
    public:
        Client();
        ~Client();
        Client(const Client& other) = delete;
        Client& operator=(const Client& other) = delete;        // int     getFlag(void) const;
        //move constructor
        Client(Client&& other) noexcept;
        Client& operator=(Client&& other) noexcept;
        bool operator==(const Client& other);
        // void    setFlag(int newState);
        // int     settingUp(int* fd);
        State getState() const;
        void setState(State newState);
        int* getClFd(void);

        int copySocketFd(int* fd);
        int handleEvent(uint32_t ev) override;
        //timeout??
};

enum State {
    WRITING,
    READING,
    CLOSE
};