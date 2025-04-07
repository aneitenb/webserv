/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/03 16:21:54 by mspasic           #+#    #+#             */
/*   Updated: 2025/04/08 00:14:38 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/WebServer.hpp"
#include <string.h>
#include <fcntl.h>

WebServer::WebServer(){}
WebServer::~WebServer(){}


int setuping(int *fd){
    int sock_err = 0;
    //setsockopt: manipulate options for the socket 
    //CONSIDER: SO_RCVBUF / SO_SNDBUF, SO_LINGER, SO_KEEPALIVE, TCP_NODELAY
    //get socket error
    if ((setsockopt(*fd, SOL_SOCKET, SO_ERROR, &sock_err, sizeof(sock_err))) == -1){
        std::cerr << "Error: setsockopt() failed: SO_ERROR: " << sock_err << "\n";
        std::cerr << strerror(errno) << "\n";
        return (-1);
    }
    //make it non-blocking
    if ((fcntl(*fd, F_SETFL, O_NONBLOCK)) == -1){
        std::cerr << "Error: fcntl() failed\n";
        std::cerr << strerror(errno) << "\n";
        return (-1);
    }     
    return (0);
}

int bind_listen(VirtualHost* cur, int* fd){
    //for the listening socket bind and listen
    std::cout << "cehcking: " << cur->getIP() << std::endl;
    if ((bind(*fd, cur->getAddress(), sizeof(struct sockaddr)) == -1)){
        std::cerr << "Error: bind() failed\n";
        std::cerr << strerror(errno) << "\n";
        return (-1);        
    }
    if ((listen(*fd, 20) == -1)){
        std::cerr << "Error: listen() failed\n";
        std::cerr << strerror(errno) << "\n";
        return (-1);   
    }
    return (0);
}

int WebServer::initialize(std::vector<ServerBlock> *_ServerBlock){
    std::vector<std::string> curPorts;
    std::size_t counter;
    std::size_t theCounter = 0;
    // int fd = -1;
    int curSockFd = -1;
    

    for (std::size_t i = 0; i < _ServerBlock->size(); i++){
        curPorts = _ServerBlock->at(i).getListen();
        counter = curPorts.size();
        std::cout << "udh " << counter << std::endl;
        theCounter += counter;
        for (std::size_t j = 0; j < counter; j++)
        {
            VirtualHost curVH(&(_ServerBlock->at(i)), curPorts.at(j));
            std::cout << "testing: " << curVH.getIP() << std::endl;
            _virtualHosts.push_back(curVH); //check if move works properly    
            if ((curSockFd = socket(PF_INET, SOCK_STREAM, 0)) == -1){
                std::cerr << "Error: socket() failed\n";
                std::cerr << strerror(errno) << "\n";
                return (-1);
            }
            std::cout << "testing: " << curVH.getIP() << std::endl;
            std::cout << "testing: " << _virtualHosts.at(0).getIP() << std::endl;
            if ((bind_listen(&_virtualHosts.at(theCounter - counter), &curSockFd)) == -1)
                return(-1);
            Listener curL(curPorts.at(j), _ServerBlock->at(i).getHost());
            curL.setSocketFd(&curSockFd);
            _listeners.push_back(curL);
        }
    }
    //go through the info from the server blocks add socket_fd
    //save stuff in virtualhost or do we even need this, maybe we just add it to the server block?
    //check for multiple hosts and save listeners
    //make listeners nonbloccking etc
    //figure out error handling
    return (0);
}

