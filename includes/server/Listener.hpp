/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Listener.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/03 15:33:58 by mspasic           #+#    #+#             */
/*   Updated: 2025/04/06 22:16:11 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>
#include <iostream>

class Listener{
    private:
        int         *_sockFd;
        std::string _port;
        std::string _host;
    public:
        Listener();
        ~Listener();
        void    acceptClient(void);

        //getters and setters
        const int getSocketFd(void);
        void    setSocketFd(int* fd); //dup?
        const std::string& getPort(void); 
        void setPort(const std::string& port);
        const std::string& getHost(void); 
        void setHost(const std::string& host);
    };