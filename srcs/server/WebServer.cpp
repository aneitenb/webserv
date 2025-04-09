/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/03 16:21:54 by mspasic           #+#    #+#             */
/*   Updated: 2025/04/09 18:22:07 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/WebServer.hpp"
#include <string.h>
#include <fcntl.h>
#include <netdb.h> //getaddrinfo

WebServer::WebServer(){}
WebServer::~WebServer(){}


int setuping(int *fd){
    // int sock_err = 0;
    //setsockopt: manipulate options for the socket 
    //CONSIDER: SO_RCVBUF / SO_SNDBUF, SO_LINGER, SO_KEEPALIVE, TCP_NODELAY
    //get socket error
    // if ((setsockopt(*fd, SOL_SOCKET, SO_ERROR, &sock_err, sizeof(sock_err))) == -1){
    //     std::cerr << "Error: setsockopt() failed: SO_ERROR: " << sock_err << "\n";
    //     std::cerr << strerror(errno) << "\n";
    //     return (-1);
    // }
    //make it non-blocking
    if ((fcntl(*fd, F_SETFL, O_NONBLOCK)) == -1){
        std::cerr << "Error: fcntl() failed\n";
        std::cerr << strerror(errno) << "\n";
        return (-1);
    }
    std::cout << "socketFD " << *fd << " has been successfully set up\n";     
    return (0);
}

int bind_listen(VirtualHost* cur, int& fd){
    //for the listening socket bind and listen
    std::cout << "cehcking: " << cur->getIP() << std::endl;
    if ((bind(fd, cur->getAddress(), sizeof(struct sockaddr)) == -1)){
        std::cerr << "Error: bind() failed\n";
        std::cerr << strerror(errno) << "\n";
        return (-1);        
    }
    if ((listen(fd, 20) == -1)){
        std::cerr << "Error: listen() failed\n";
        std::cerr << strerror(errno) << "\n";
        return (-1);   
    }
    return (0);
}

int WebServer::initialize(std::vector<ServerBlock>& serBlocks){
    std::vector<std::string> curPorts;
    std::size_t maxPorts;
    std::size_t countVH = 0;
    // int fd = -1;
    int curSockFd = -1;

    for (std::size_t countS = 0; countS < serBlocks.size(); countS++){
        std::cout << "serverNamesCheck " << serBlocks.at(countS).getServerName() << std::endl;
        curPorts = serBlocks.at(countS).getListen();
        maxPorts = curPorts.size();
        std::cout << "counter " << maxPorts << std::endl;
        std::cout << "size oof serv vector " << serBlocks.size() << std::endl;
        std::cout << "i is " << countS << std::endl;
        for (std::size_t countP = 0; countP < maxPorts; countP++)
        {
            // std::cout << "checking with 1 port\n";
            VirtualHost curVH(serBlocks.at(countS), curPorts.at(countP));
            if (curVH.addressInfo() == -1)
                return (-1);
            std::cout << "testing: " << curVH.getIP() << std::endl;
            _virtualHosts.push_back(std::move(curVH)); //check if move works properly    
            if ((curSockFd = socket(PF_INET, SOCK_STREAM, 0)) == -1){
                std::cerr << "Error: socket() failed\n";
                std::cerr << strerror(errno) << "\n";
                return (-1);
            }
            if ((setuping(&curSockFd)) == -1)
                return (-1);
            std::cout << "dsocketfd " << curSockFd << std::endl;
            if ((bind_listen(&_virtualHosts.at(countVH), curSockFd)) == -1)
                return(-1);
            Listener curL(curPorts.at(countP), serBlocks.at(countS).getHost());
            if (curL.setSocketFd(curSockFd) == -1)
                return (-1);
            close (curSockFd);
            curSockFd = -1;
            _virtualHosts.at(countVH).setup_fd(curL.getSocketFd()); 
            _listeners.push_back(curL);
            std::cout << "listener testing: " << _listeners.at(countVH).getPort() << std::endl;
            countVH++;
        }
    }
    //go through the info from the server blocks add socket_fd
    //save stuff in virtualhost or do we even need this, maybe we just add it to the server block?
    //check for multiple hosts and save listeners
    //make listeners nonbloccking etc
    //figure out error handling
    return (0);
}


void WebServer::freeStuff(void){
    for (std::size_t i = 0; i < _virtualHosts.size(); i++){
        freeaddrinfo(_virtualHosts.at(i).getRes());
    }
}

std::vector<Listener> WebServer::getListeners(void) const{
    return (_listeners);
}

std::vector<VirtualHost> WebServer::getVHosts(void) const{
    return (_virtualHosts);
}
