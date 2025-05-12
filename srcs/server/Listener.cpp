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
#include <netdb.h> //getaddrinfo
#include <cstring> //memset


/*Constructors and Destructor and Operators*/
Listener::Listener() : _sockFd(-1){};

Listener::Listener(std::string port, std::string host) : _sockFd(-1), _port(port), _host(host) {}

Listener::Listener(const Listener& obj){
    _sockFd = obj._sockFd;
    // this->copySocketFd(obj._sockFd);
    _port = obj._port;
    _host = obj._host;
    _allServerNames = obj._allServerNames;
    // _relevant = obj._relevant;
}
//remember to close the previous fd
Listener& Listener::operator=(const Listener& obj) {
    if (this != &obj){
        // this->copySocketFd(obj._sockFd);
        _sockFd = obj._sockFd;
        _port = obj._port;
        _host = obj._host;
        _allServerNames = obj._allServerNames;
        // _relevant = obj._relevant;
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

/*Getters and Setters*/

// std::vector<VirtualHost> Listener::getHosts(void) const{
//     return (_knownVHs);
// }

// void Listener::addHost(VirtualHost& cur){
//     _knownVHs.push_back(cur);
// }

void Listener::setFd(int fd){
    _sockFd = fd;
}

const std::string& Listener::getFirstKey() const{
    return (_firstKey);
}

void Listener::setFirst(std::string key){
    _firstKey = key;
}

void Listener::addServBlock(ServerBlock& cur, std::string name){
    _allServerNames[name] = cur;
    // _relevant = cur;
}

std::unordered_map<std::string, ServerBlock> Listener::getServBlock(){
    return (_allServerNames);
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

// struct addrinfo* Listener::getRes() const{
//     return(_result);
// }

struct sockaddr* Listener::getAddress() const{
    return(_result->ai_addr);
}

// socklen_t Listener::getAddressLength() const{
//     return(_result->ai_addrlen);
// }

void Listener::freeAddress(void){
    if (_result){
        freeaddrinfo(_result);
        _result = nullptr;
    }
}
        
void Listener::cleanResult(void){
    _result = nullptr;
}

/*Helper functions*/
int Listener::addressInfo(void){
    struct addrinfo hints;
    int status;

    ftMemset(&hints, sizeof(hints));
    hints.ai_family = AF_INET; //IPv4
    hints.ai_socktype = SOCK_STREAM; //TCP
    hints.ai_flags = AI_PASSIVE; //for binding (listening) maybe not needed if we always provide an IP or hostname
    if ((status = getaddrinfo(_host.c_str(), _port.c_str(), &hints, &_result)) != 0){
        std::cerr << "Error: getaddrinfo() failed: ";
        if (status == EAI_SYSTEM)
            std::cerr << strerror(errno) << "\n";
        else
            std::cerr << gai_strerror(status) << "\n";
        return (-1);
    }
    if (_result && _result->ai_family != AF_INET){
        std::cerr << "Error: getaddrinfo failed but unclear why.\n";
        return (-1);
    }
    std::cout << "Listener address set up!\n";
    return (0);
}

int Listener::makeNonBlock(int* fd){
    if ((fcntl(*fd, F_SETFL, O_NONBLOCK)) == -1){
        std::cerr << "Error: fcntl() failed\n";
        std::cerr << strerror(errno) << "\n";
        return (-1);
    }
    return (0);
}

int Listener::setuping(int *fd){
    // int sock_err = 0;
    //setsockopt: manipulate options for the socket 
    //CONSIDER: SO_RCVBUF / SO_SNDBUF, SO_LINGER, SO_KEEPALIVE, TCP_NODELAY
    //get socket error
    // if ((setsockopt(*fd, SOL_SOCKET, SO_ERROR, &sock_err, sizeof(sock_err))) == -1){
    //     std::cerr << "Error: setsockopt() failed: SO_ERROR: " << sock_err << "\n";
    //     std::cerr << strerror(errno) << "\n";
    //     return (-1);
    // }
    /*TEST TO SEE IF THE NONBLOCKING HAS BEEN SET UP FOR THE LISTENER, DELETE AFTER*/
    if (makeNonBlock(fd) == -1)
        return (-1);
    if ((bind(*fd, this->getAddress(), sizeof(struct sockaddr)) == -1)){
        std::cerr << "Error: bind() failed\n";
        std::cerr << strerror(errno) << "\n";
        return (-1);        
    }
    if ((listen(*fd, 20) == -1)){
        std::cerr << "Error: listen() failed\n";
        std::cerr << strerror(errno) << "\n";
        return (-1);   
    }
    // // After socket(), before bind()/listen():
    // int flags = fcntl(*fd, F_GETFL, 0);
    // if (flags < 0) {
    //     perror("fcntl F_GETFL");
    //     exit(1);
    // }

    // // Turn on non-blocking if it wasn’t already:
    // if (!(flags & O_NONBLOCK)) {
    //     if (fcntl(*fd, F_SETFL, flags | O_NONBLOCK) < 0) {
    //         perror("fcntl F_SETFL O_NONBLOCK");
    //         exit(1);
    //     }
    //     printf("enabled O_NONBLOCK on fd %d\n", *fd);
    // }

    // // Verify:
    // flags = fcntl(*fd, F_GETFL, 0);
    // printf("listen_fd access mode: ");
    // switch (flags & O_ACCMODE) {
    // case O_RDONLY:  printf("read-only");  break;
    // case O_WRONLY:  printf("write-only"); break;
    // case O_RDWR:    printf("read/write"); break;
    // default:        printf("unknown");    break;
    // }
    // printf("\nstatus flags: %sO_NONBLOCK\n",
    //     (flags & O_NONBLOCK) ? "" : "(!) ");
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
    if (this->addressInfo() == -1)
        return (-1);
    std::cout << "This socket has the fd: " << _sockFd << std::endl;
    if ((setuping(&_sockFd)) == -1)
        return (-1);
    int status = fcntl(_sockFd, F_GETFD); //delete
    if (status == -1) {
        perror("File descriptor is not valid");
    }
    return (0);
}

// int Listener::copySocketFd(const int& fd){
//         if (_sockFd != -1){
//             close(_sockFd);
//             _sockFd = -1;}
//         if (fd == -1)
//             return (_sockFd);
//         _sockFd = dup(fd);
//         if (_sockFd == -1)
//             std::cout << "Error: dup() failed\n";
//         // close(fd);
//         return (_sockFd);
// }

void Listener::addClient(Client& cur){
    _activeClients.push_back(std::move(cur));
}

// void Listener::addServName(std::string name){
//     if (_allServerNames.empty() != true){
//         if (_allServerNames.count(name) > 0)
//             return;
//     }
//     _allServerNames[name];
// }


const std::vector<Client>& Listener::getClients(void) const{
    return (_activeClients);
}

void Listener::delClient(Client* cur){
    if (!cur)
        return ;
    for (std::size_t i = 0; i < _activeClients.size(); i++){
        if (_activeClients.at(i) == *cur){
            _activeClients.erase(_activeClients.begin() + i);
            return ;}
    }
}

/*Overriden*/
int* Listener::getSocketFd(void){
    return(&_sockFd);
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
                    return (0); //means there are no more clients that wait to be accepted
                std::cerr << "Error: accept() failed: ";
                std::cerr << strerror(errno) << "\n";
                return (-1);
            }
            //separate setuping into making it nonblocking
            if (setuping(&curFd) == -1) //set as nonblocking
                return (-1);
            if (curC.setFd(curFd) == -1) //pass the socket into Client
                return (-1);
            _activeClients.push_back(std::move(curC));
            curFd = -1; //i think this won't keep the fds open haha
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

EventHandler* Listener::getCgi() { return {}; }

bool Listener::conditionMet(std::unordered_map<int*, std::vector<EventHandler*>>& _activeFds, int& epollFd) { 
    (void)_activeFds;
    (void)epollFd;
    return false; 
}

struct epoll_event& Listener::getCgiEvent(int flag) { 
    (void)flag;
    return (*this->getEvent()); //wont be used
}

bool Listener::ready2Switch() { return false; }
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