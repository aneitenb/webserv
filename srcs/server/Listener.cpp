// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<Listener.cpp>> -- <<Aida, Ilmari, Milica>>

#include "server/Listener.hpp"
#include "server/Client.hpp"
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

Listener::Listener() : _sockFd(-1){};

Listener::Listener(std::string port, std::string host) : _sockFd(-1), _port(port), _host(host) {}

// Listener::Listener() : _sockFd(-1){};

Listener::Listener(const Listener& obj){
    _sockFd = -1;
    this->copySocketFd(obj._sockFd);
    _port = obj._port;
    _host = obj._host;
    _relevant = obj._relevant;
}
//remember to close the previous fd
Listener& Listener::operator=(const Listener& obj) {
    if (this != &obj){
        this->copySocketFd(obj._sockFd);
        _port = obj._port;
        _host = obj._host;
        _relevant = obj._relevant;
    }
    return (*this);
}

Listener::~Listener(){
    if (_sockFd != -1){
        close(_sockFd);
        _sockFd = -1;
        // std::cout << "closed fd\n";
    }
}

int* Listener::getSocketFd(void){
    return(&_sockFd);
}

/*setting up the listening (and client?) socket to nonblocking*/
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
    // std::cout << "socketFD " << *fd << " has been successfully set up as non-blocking\n";     
    return (0);
}

int Listener::setSocketFd(void){
    if (_sockFd != -1){
        close(_sockFd);
        _sockFd = -1;}
    if ((_sockFd = socket(PF_INET, SOCK_STREAM, 0)) == -1){
        std::cerr << "Error: socket() failed\n";
        std::cerr << strerror(errno) << "\n";
        return (-1);
    }
    if ((setuping(&_sockFd)) == -1)
        return (-1);
    return (0);
}

int Listener::copySocketFd(const int& fd){
        if (_sockFd != -1){
            close(_sockFd);
            _sockFd = -1;}
        if (fd == -1)
            return (_sockFd);
        _sockFd = dup(fd);
        if (_sockFd == -1)
            std::cout << "Error: dup() failed\n";
        // close(fd);
        return (_sockFd);
}

// std::vector<VirtualHost> Listener::getHosts(void) const{
//     return (_knownVHs);
// }

// void Listener::addHost(VirtualHost& cur){
//     _knownVHs.push_back(cur);
// }

void Listener::addServBlock(ServerBlock& cur){
    _relevant = cur;
}

ServerBlock* Listener::getServBlock(){
    return (&_relevant);
}

const std::string& Listener::getPort(void) const{
    return (_port);
}


void Listener::setPort(const std::string& port){
    _port = port;
}

        
const std::string& Listener::getHost(void) const{
    return (_host);
}

void Listener::setHost(const std::string& host){
    _host = host;
}

int Listener::handleEvent(uint32_t ev){
    if (ev & EPOLLERR || ev & EPOLLHUP){
        //rare but it could happen
        //in the case of err, socket is unusable
        //in the case of hup, socket is hanging
        std::cerr << "Fatal error occurred with the socket. Shutting down.\n";
        return (-1); //cleanup
    }
    if (ev & EPOLLIN){ //accept incoming clients while there are clients to be accepted
        while (1){
            Client curC(this->getServBlock());
            int curFd = -1;
            curFd = accept(_sockFd, nullptr, nullptr); //think about taking in the client info for security reasons maybe
            if (curFd == -1){
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                    return (0); //means there are no more clients that wait to be accepted, but can we use errno??
                std::cerr << "Error: accept() failed: ";
                std::cerr << strerror(errno) << "\n";
                return (-1);
            }
            if (setuping(&curFd) == -1) //set as nonblocking
                return (-1);
            if (curC.copySocketFd(&curFd) == -1) //pass the socket into Client
                return (-1);
            // EventLoop* curEL = &(this->getLoop()); //get the EventLoop
            // curC.setLoop(*curEL); //set the EventLoop for the client
            // if (curEL->addToEpoll(curC.getClFd(), EPOLLIN, &curC) == -1)
            //     return (-1); //add the client fd to the epoll
            _activeClients.push_back(std::move(curC));
            closeFd(&curFd);
            // curEL->addClient(&(_activeClients.at(_activeClients.size() - 1)));
        }
    }
    return (0);
}

std::vector<EventHandler*> Listener::resolveAccept(void) {
    std::vector<EventHandler*> clients;

    clients.reserve(_activeClients.size());

    for (Client& client : _activeClients)
        clients.push_back(&client);
        
    return(clients);
}

void Listener::resolveClose(){
    if (_activeClients.size() == 0)
        return ;
    ssize_t i = 0;
    for (auto it = _activeClients.begin(); it != _activeClients.end(); ){
        if (_activeClients.at(i).getState() == TOCLOSE){
            _activeClients.at(i).setState(CLOSED);
            closeFd(_activeClients.at(i).getSocketFd());
            it = _activeClients.erase(it); }// erase returns the next valid iterator
        else {
            ++it;
            i++;
        }
    }
}

void Listener::addClient(Client& cur){
    _activeClients.push_back(std::move(cur));
}

const std::vector<Client>& Listener::getClients(void) const{
    return (_activeClients);
}

void Listener::delClient(Client* cur){
    for (std::size_t i = 0; i < _activeClients.size(); i++){
        if (_activeClients.at(i) == *cur){
            _activeClients.erase(_activeClients.begin() + i);
            return ;}
    }
}


//FOR LISTENERS
// void EventLoop::addClient(Client* cur){
//     _activeClients.push_back(cur);
// }

// std::vector<Client*> EventLoop::getClients(void) const{
//     return (_activeClients);
// }

// void EventLoop::delClient(Client* cur){
//     for (std::size_t i = 0; i < _activeClients.size(); i++){
//         if (_activeClients.at(i) == cur){
//             _activeClients.erase(_activeClients.begin() + i);
//             return ;}
//     }
// }