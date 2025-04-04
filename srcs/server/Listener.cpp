/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Listener.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/04 15:10:44 by mspasic           #+#    #+#             */
/*   Updated: 2025/04/04 15:32:15 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Listener.hpp"

Listener::Listener(){
    _sockFd = nullptr;
}

Listener::~Listener(){
    _sockFd = nullptr;
}

const int Listener::getSocketFd(void){
    return(*_sockFd);
}

void Listener::setSocketFd(int &fd){
    *_sockFd = fd;
}


const std::string& Listener::getPort(void){
    return (_port);
}


void Listener::setPort(const std::string& port){
    _port = port;
}

        
const std::string& Listener::getHost(void){
    return (_host);
}

void Listener::setHost(const std::string& host){
    _host = host;
}

void Listener::acceptClient(void){
    //code
}