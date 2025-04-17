// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<WebServer.cpp>> -- <<Aida, Ilmari, Milica>>

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
    std::cout << "testing fd set up: " << cur->getFD() << std::endl;
    return (0);
}

bool WebServer::doesExist(std::string port, std::string host){
    std::cout << "entered doesExist\n";
    if (_theSList.count(port) == FALSE){
        std::cout << "this shouldve happened\n";
        _theSList[port].push_back(host);
        return FALSE;
    }
    if (_theSList.at(port).empty() == FALSE){
        for (size_t i = 0; i < _theSList.at(port).size(); i++)
            if (_theSList.at(port).at(i) == port)
                return TRUE;
    }
    _theSList.at(port).push_back(host);
    return TRUE;
}

/*check if the [port : host] combination exists and add it if it doesn't*/
bool WebServer::doesExistPort(std::string port){
    std::cout << "entered doesExistPort\n";
    return (_theSList.count(port));
}

int WebServer::resolveListener(std::string port, std::string host){
    if (doesExistPort(port) == FALSE){
        std::cout << "this shouldve happened\n";
        Listener curL(port, host);
        if (curL.setSocketFd() == -1)
            return (-1);
        _theLList.push_back(curL);
        curL.closeFd(curL.getSocketFd());
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
    int countL = 0;

    for (std::size_t countS = 0; countS < serBlocks.size(); countS++){
        std::cout << "serverNamesCheck " << serBlocks.at(countS).getServerName() << std::endl;
        curPorts = serBlocks.at(countS).getListen();
        maxPorts = curPorts.size();
        curHost = serBlocks.at(countS).getHost();
        for (std::size_t countP = 0; countP < maxPorts; countP++){
            if ((countL = this->resolveListener(curPorts.at(countP), curHost)) == -1)
                return (-1);
            VirtualHost curVH(serBlocks.at(countS), curPorts.at(countP));
            if (curVH.addressInfo() == -1)
                    return (-1);
            if (doesExist(curPorts.at(countP), curHost) == FALSE){
                if ((bind_listen(&curVH, _theLList.at(countL).getSocketFd())) == -1)
                        return(-1);
            }
            _theVHList[_theLList.at(countL).getSocketFd()].push_back(std::move(curVH));
            std::cout << "testing: " << curVH.getIP() << std::endl;
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
    for (auto& pair : _theVHList) {
        std::vector<VirtualHost>& curVH = pair.second;
    
        for (auto& obj : curVH) {
            obj.freeAddress();
        }
    }
}

std::vector<Listener>& WebServer::getListeners(void){
    return (_theLList);
}

// std::vector<VirtualHost> WebServer::getVHosts(void) const{
//     return (_virtualHosts);
// }
