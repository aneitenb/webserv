/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Listener.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/03 15:33:58 by mspasic           #+#    #+#             */
/*   Updated: 2025/04/07 15:42:19 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>
#include <iostream>

class Listener{
    private:
        int         _sockFd;
        std::string _port;
        std::string _host;
        Listener() = default;
    public:
        Listener(std::string _port, std::string _host);
        ~Listener();
        Listener(Listener&& obj);
        void    acceptClient(void);

        //getters and setters
        int getSocketFd(void) const;
        int    setSocketFd(int* fd); //dup?
        const std::string& getPort(void) const; 
        void setPort(const std::string& port);
        const std::string& getHost(void) const; 
        void setHost(const std::string& host);
    };