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
#include "utils/message.hpp"
#include "server/WebServer.hpp"

WebServer::WebServer(){}
WebServer::~WebServer(){}

bool WebServer::doesListenerExist(std::string port){
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

// Create one listener per port, add all server blocks to it
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
        // Create new listener that binds to wildcard (0.0.0.0)
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
