/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/03 15:36:48 by mspasic           #+#    #+#             */
/*   Updated: 2025/04/06 23:49:52 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <sys/epoll.h>

class Client{
    private:
        int*                _listfd;
        int                 _clFd;
        int                 _state;
        struct sockaddr*    _result;
        struct epoll_event  _event;
        Client obj(const Client& other) = delete;
        Client& operator=(const Client& other) = delete;
    public:
        Client();
        ~Client();
        int     getFlag(void) const;
        void    setFlag(int newState);
        int     settingUp(int* fd);
};