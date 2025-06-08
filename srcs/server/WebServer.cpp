// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<WebServer.cpp>> -- <<Aida, Ilmari, Milica>>

#include <string.h>
#include <netdb.h> //getaddrinfo
#include <fcntl.h> //delete after
#include "log.hpp"

#include "utils/message.hpp"
#include "server/WebServer.hpp"

WebServer::WebServer(){}
WebServer::~WebServer(){}



/*use bind and listen for listening sockets*/
// int WebServer::bind_listen(Listener* cur, int* fd){
//     //for the listening socket bind and listen
//     getLogFile() << "fd is: " << *fd << " and the host is " << cur->getAddress()->sa_family << std::endl;
//     int status = fcntl(*fd, F_GETFD); //delete
//     if (status == -1) {
//         perror("File descriptor is not valid");
//     }
//     if ((bind(*fd, cur->getAddress(), sizeof(struct sockaddr)) == -1)){
//         std::cerr << "Error: bind() failed\n";
//         std::cerr << strerror(errno) << "\n";
//         return (-1);        
//     }
//     if ((listen(*fd, 20) == -1)){
//         std::cerr << "Error: listen() failed\n";
//         std::cerr << strerror(errno) << "\n";
//         return (-1);   
//     }
//     cur->setup_fd(fd);
//     getLogFile() << "testing fd set up: " << cur->getFD() << std::endl;
//     return (0);
// }

// bool WebServer::doesExist(std::string port, std::string host){ //check
//     getLogFile() << "entered doesExist\n";
//     if (_theSList.count(port) == 0){
//         getLogFile() << "this shouldve happened\n";
//         _theSList[port].push_back(host);
//         return false;
//     }
//     if (_theSList.at(port).empty() == true){
//         for (size_t i = 0; i < _theSList.at(port).size(); i++)
//             if (_theSList.at(port).at(i) == port)
//                 return true;
//     }
//     _theSList.at(port).push_back(host);
//     return true;
// }


// CHANGED: Now checks only for port (not port+host combo)
bool WebServer::doesListenerExist(std::string port){
    // check if we already have a listener for this port
    for (std::size_t i = 0; i < _theLList.size(); i++){
        if (_theLList.at(i).getPort() == port) {
            return true;
        }
    }
    return false;
}

Listener* WebServer::findListenerByPort(std::string port){
    for (std::size_t i = 0; i < _theLList.size(); i++){
        if (_theLList.at(i).getPort() == port) {
            return &_theLList.at(i);
        }
    }
    return nullptr;
}

std::string WebServer::createUniqueKey(const std::string& host, const std::string& port, const std::string& serverName) {
    if (serverName.empty()) {
        // if no server name, use host:port
        return host + ":" + port;
    } else {
        // Use server_name@host:port
        return serverName + "@" + host + ":" + port;
    }
}

// CHANGED: Create one listener per port, add all server blocks to it
int WebServer::resolveListener(std::string port, std::string host, ServerBlock& serBlock){
    // Check if we already have a listener for this port
    if (doesListenerExist(port)) {
		Debug("Found existing listener for port " << port);
        // Find the existing listener and add this server block to it
        Listener* existingListener = findListenerByPort(port);
        if (existingListener) {
            std::string key = createUniqueKey(host, port, serBlock.getServerName());
            
            existingListener->addServBlock(serBlock, key);
			Debug("Added server block with key " << key << " to listener for port " << port);
        }
    } else {
        // create new listener that binds to wildcard (0.0.0.0)
        Listener curL(port, "0.0.0.0");
        
        std::string key = createUniqueKey(host, port, serBlock.getServerName());
        curL.addServBlock(serBlock, key);
        
        if (curL.setSocketFd() == -1){
            curL.freeAddress();
            return (-1);
        }
        
        _theLList.push_back(std::move(curL));
		Debug("Created listener for port " << port << " with key " << key << " on socket #" << *(_theLList.back().getSocketFd(0)) << '\n');
    }
    return (0);
}

// CHANGED: Updated to work with new logic
int WebServer::initialize(std::vector<ServerBlock>& serBlocks){
    std::vector<std::string> curPorts;
    std::size_t maxPorts;
    std::string curHost;

    for (std::size_t countS = 0; countS < serBlocks.size(); countS++){
        curPorts = serBlocks.at(countS).getListen();
        maxPorts = curPorts.size();
        curHost = serBlocks.at(countS).getHost();
        
        for (std::size_t countP = 0; countP < maxPorts; countP++){
            if (this->resolveListener(curPorts.at(countP), curHost, serBlocks.at(countS)) == -1)
                return (-1);
        }
    }
    return (0);
}


void WebServer::freeStuff(void){
    for (std::size_t i = 0; i < _theLList.size(); i++)
        _theLList.at(i).freeAddress();
}

std::vector<Listener>& WebServer::getListeners(void){
    return (_theLList);
}

// std::vector<VirtualHost> WebServer::getVHosts(void) const{
//     return (_virtualHosts);
// }
