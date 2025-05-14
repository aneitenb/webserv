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
#include <fcntl.h> //delete after

WebServer::WebServer(){}
WebServer::~WebServer(){}



/*use bind and listen for listening sockets*/
// int WebServer::bind_listen(Listener* cur, int* fd){
//     //for the listening socket bind and listen
//     std::cout << "fd is: " << *fd << " and the host is " << cur->getAddress()->sa_family << std::endl;
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
//     std::cout << "testing fd set up: " << cur->getFD() << std::endl;
//     return (0);
// }

// bool WebServer::doesExist(std::string port, std::string host){ //check
//     std::cout << "entered doesExist\n";
//     if (_theSList.count(port) == 0){
//         std::cout << "this shouldve happened\n";
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

/*check if the [port : host] combination exists and add it if it doesn't*/
bool WebServer::doesListenerExist(std::string port, std::string host){
    // std::cout << "entered doesExistPort\n";
    if (_portHost.count(port) > 0){
        std::cout << "Found port: " << _portHost.count(port) << std::endl;
        for (std::size_t i = 0; i < _portHost.at(port).size(); i++){
            if (_portHost.at(port).at(i) == host)
                return true;
        }
    }
    _portHost[port].push_back(host);
    return false;
}

int WebServer::resolveListener(std::string port, std::string host, ServerBlock& serBlock){
    //check if port exists, if yes go to the next check
    if (doesListenerExist(port, host) == true) //exists
    {
        std::cout << "Apparently, the port host combo exists\n";
        for (std::size_t j = 0; j < _theLList.size(); j++){
            if (_theLList.at(j).getHost() == host)
                _theLList.at(j).addServBlock(serBlock, serBlock.getServerName()); //add down too
        }
    }
    else{
        std::cout << "PortHost doesn't exits, adding\n";
        Listener curL(port, host);
        curL.addServBlock(serBlock, serBlock.getServerName());
        // curL.setFirst(serBlock.getServerName());
        if (curL.setSocketFd() == -1){
            curL.freeAddress();
            return (-1);}
        _theLList.push_back(std::move(curL));
        std::cout << *(_theLList.back().getSocketFd()) << "    this is the listeners true fd\n";
        std::cout << *(curL.getSocketFd()) << "       this is the fd you want to close\n\n";
    }
    return (0);
}

//shorten?
int WebServer::initialize(std::vector<ServerBlock>& serBlocks){
    std::vector<std::string> curPorts; //can have more than one port in a server block
    std::size_t maxPorts;
    std::string curHost; //IP
    // int countL = 0;

    for (std::size_t countS = 0; countS < serBlocks.size(); countS++){ //counter for serverblocks
        // std::cout << "serverNamesCheck " << serBlocks.at(countS).getServerName() << std::endl;
        curPorts = serBlocks.at(countS).getListen(); //get all the ports from the server block
        // std::cout << "checking the first port in a server block " << curPorts.at(0) << std::endl;
        maxPorts = curPorts.size(); //get the number of ports in the server block
        // std::cout << "how many ports in a server block " <<curPorts.size() << std::endl;
        curHost = serBlocks.at(countS).getHost(); //get the current IP address
        std::cout << "Cur host: " << curHost << std::endl;
        for (std::size_t countP = 0; countP < maxPorts; countP++){
            if (this->resolveListener(curPorts.at(countP), curHost, serBlocks.at(countS)) == -1)
                return (-1);
            // _theVHList[_theLList.at(countL).getSocketFd()].push_back(std::move(curVH));
            // std::cout << "testing: " << curVH.getIP() << std::endl;
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
    for (std::size_t i = 0; i < _theLList.size(); i++) {
        _theLList.at(i).freeAddress();
        std::cout << "Did the free\n";
    }
}

std::vector<Listener>& WebServer::getListeners(void){
    return (_theLList);
}

// std::vector<VirtualHost> WebServer::getVHosts(void) const{
//     return (_virtualHosts);
// }
