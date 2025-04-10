/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Listener.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/03 15:33:58 by mspasic           #+#    #+#             */
/*   Updated: 2025/04/10 23:38:36 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <map>
#include <iostream>
#include <vector>
#include "VirtualHost.hpp"

class Listener{
    private:
        int         _sockFd; //before every use check?
        std::vector<VirtualHost> _knownVHs;
        std::string _port;
        std::string _host;
    public:
        Listener();
        Listener(std::string _port, std::string _host);
        ~Listener();
        Listener(const Listener& other);
        Listener& operator=(const Listener& other);
        //move constructor and move assignment operator
        // Listener(Listener&& obj) noexcept;
        // Listener& operator=(Listener&& obj) noexcept;

        void    acceptClient(void);

        //getters and setters
        int* getSocketFd(void);
        int    setSocketFd(void); 
        int copySocketFd(const int& fd);//dup?
        std::vector<VirtualHost> getHosts(void) const;
        void addHost(VirtualHost& cur);
        const std::string& getPort(void) const; 
        void setPort(const std::string& port);
        const std::string& getHost(void) const; 
        void setHost(const std::string& host);
        void closeFD(void);
    };