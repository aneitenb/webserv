/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/03 16:21:54 by mspasic           #+#    #+#             */
/*   Updated: 2025/04/10 22:30:14 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server/WebServer.hpp"
#include <string.h>
#include <netdb.h> //getaddrinfo

WebServer::WebServer(){}
WebServer::~WebServer(){}



/*use bind and listen for listening sockets*/
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
    cur->setup_fd(fd);
    return (0);
}

bool WebServer::doesExist(std::string port, std::string host){
    if (_theSList.count(port) == TRUE){
        if (_theSList.at(port).empty() == TRUE){
            _theSList.at(port).push_back(host);
            return TRUE;
        }
        for (size_t i = 0; i < _theSList.at(port).size(); i++){
            if (_theSList.at(port).at(i) == port)
                return TRUE;
        }
        _theSList.at(port).push_back(host);
        return TRUE;
    }
    _theSList[port].push_back(host);
    return FALSE;
}

/*check if the [port : host] combination exists and add it if it doesn't*/
bool WebServer::doesExistPort(std::string port){
    return (_theSList.count(port));
}

std::size_t WebServer::resolveListener(std::string port, std::string host){
    if (doesExistPort(port) == FALSE){
        Listener curL(port, host);
        if (curL.setSocketFd() == -1)
            return (-1);
        _theLList.push_back(curL);
        return (_theLList.size() - 1);
    }
    for(std::size_t m = 0; m < _theLList.size(); m++){
        if (_theLList.at(m).getPort() == port)
            return (m);
    }
    return (-1);
}

//shorten?
int WebServer::initialize(std::vector<ServerBlock>& serBlocks){
    std::vector<std::string> curPorts;
    std::size_t maxPorts;
    std::string curHost;
    std::size_t countL = 0;

    for (std::size_t countS = 0; countS < serBlocks.size(); countS++){
        std::cout << "serverNamesCheck " << serBlocks.at(countS).getServerName() << std::endl;
        curPorts = serBlocks.at(countS).getListen();
        maxPorts = curPorts.size();
        //tell Aida to set up the serv name as the host if there is no IP; default IP
        curHost = serBlocks.at(countS).getHost();
        for (std::size_t countP = 0; countP < maxPorts; countP++)
        {
            if (doesExist(curPorts.at(countP), curHost) == TRUE)
                continue;
            if ((countL = this->resolveListener(curPorts.at(countP), curHost)) == -1)
                return (-1);
            VirtualHost curVH(serBlocks.at(countS), curPorts.at(countP));
            if (curVH.addressInfo() == -1)
                    return (-1);
            if ((bind_listen(&curVH, _theLList.at(countL).getSocketFd())) == -1)
                    return(-1);
            _theVHList[_theLList.at(countL).getSocketFd()].push_back(std::move(curVH));
            // std::cout << "checking with 1 port\n"; 
            std::cout << "testing: " << curVH.getIP() << std::endl;
            //add to the List instead here, but see if port exists first
            }
        }
    return (0);
}
//go through the info from the server blocks add socket_fd
//save stuff in virtualhost or do we even need this, maybe we just add it to the server block?
//check for multiple hosts and save listeners
//make listeners nonbloccking etc
//figure out error handling


void WebServer::freeStuff(void){
    for (std::size_t i = 0; i < _virtualHosts.size(); i++){
        freeaddrinfo(_virtualHosts.at(i).getRes());
    }
}

std::vector<Listener> WebServer::getListeners(void) const{
    return (_theLList);
}

std::vector<VirtualHost> WebServer::getVHosts(void) const{
    return (_virtualHosts);
}
