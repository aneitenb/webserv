/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/03 16:21:54 by mspasic           #+#    #+#             */
/*   Updated: 2025/04/06 23:23:42 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"
#include <string.h>
#include <fcntl.h>

WebServer::WebServer(){}
WebServer::~WebServer(){}


int setuping(int *fd){
    int sock_err;
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

int bind_listen(VirtualHost& cur, int* fd){
    //for the listening socket bind and listen
    if ((bind(*fd, cur.getAddress(), sizeof(struct sockaddr)) == -1)){
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

int WebServer::initialize(std::vector<ServerBlock> _ServerBlock){
    std::vector<std::string> curPorts;
    std::size_t counter;
    int fd = -1;
    int curSockFd = -1;

    for (std::size_t i = 0; i < _ServerBlock.size(); i++){
        curPorts = _ServerBlock.at(i).getListen();
        counter = curPorts.size();
        for (std::size_t j = 0; j < counter; j++)
        {
            VirtualHost curVH(_ServerBlock.at(i), curPorts.at(j));
            _virtualHosts.push_back(curVH); //check if move works properly    
            if ((curSockFd = socket(PF_INET, SOCK_STREAM, 0)) == -1){
                std::cerr << "Error: socket() failed\n";
                std::cerr << strerror(errno) << "\n";
                return (-1);
            }
            if ((bind_listen(curVH, &curSockFd)) == -1)
                return(-1);
            Listener curL(curPorts.at(j), _ServerBlock.at(i).getHost());
            curL.setSocketFd(&curSockFd);
            _listeners.push_back(curL);
        }
    }
    //go through the info from the server blocks add socket_fd
    //save stuff in virtualhost or do we even need this, maybe we just add it to the server block?
    //check for multiple hosts and save listeners
    //make listeners nonbloccking etc
    //figure out error handling
}

