// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<VirtualHost.cpp>> -- <<Aida, Ilmari, Milica>>

#include "server/VirtualHost.hpp"
#include "CommonFunctions.hpp"

// #include <sys/socket.h>
// #include <netinet/in.h>
#include <unistd.h> //close
#include <fcntl.h> //fcntl
#include <netdb.h> //getaddrinfo
// #include <poll.h>
#include <cstring> //memset
#include <iostream>

int VirtualHost::addressInfo(void){
    struct addrinfo hints;
    int status;

    ftMemset(&hints, sizeof(hints));
    hints.ai_family = AF_INET; //IPv4
    hints.ai_socktype = SOCK_STREAM; //TCP
    hints.ai_flags = AI_PASSIVE; //for binding (listening) maybe not needed if we always provide an IP or hostname
    if ((status = getaddrinfo(_IP.c_str(), _port.c_str(), &hints, &_result)) != 0){
        std::cerr << "Error: getaddrinfo() failed: ";
        if (status == EAI_SYSTEM)
            std::cerr << strerror(errno) << "\n";
        else
            std::cerr << gai_strerror(status) << "\n";
        return (-1);
    }
    if (_result->ai_family != AF_INET){
        std::cerr << "Error: getaddrinfo failed but unclear why.\n";
        return (-1);
    }
    // std::cout << "Virtual Host address set up!\n";
    return (0);
}

VirtualHost::VirtualHost() : _sockfd(nullptr), _result(nullptr){}

//check if you have copy constructors everywhere since you use vectors; push_back() copies/moves objects
VirtualHost::VirtualHost(const ServerBlock& info, const std::string& port){
    _info = info;
    _port = port;
    _IP = info.getHost();
    _serv_name = info.getServerName();
    _sockfd = nullptr;
    _sock_err = 0; //do I leave it like this?
    // ftMemset(&_result, sizeof(struct addrinfo));
    _result = nullptr;
    // memset(_result, 0, sizeof(struct addrinfo));
    // ftMemset(&_event, sizeof(_event)); //do I leave this like this?
}

VirtualHost::VirtualHost(const VirtualHost& other) {
    _info = other._info;
    _result = other._result;
    _port = other._port;
    _IP = other._IP;
    _serv_name = other._serv_name;
    _sockfd = other._sockfd; //issues?
    _sock_err = other._sock_err;
    // _event = other._event;
}

VirtualHost& VirtualHost::operator=(const VirtualHost& other) {
    if (this != &other){
        _info = other._info;
        _result = other._result;
        _port = other._port;
        _IP = other._IP;
        _serv_name = other._serv_name;
        _sockfd = other._sockfd; //issues?
        _sock_err = other._sock_err;
        // _event = other._event;
    }
    return (*this);
}


VirtualHost::VirtualHost(VirtualHost&& other) noexcept : _info(other._info), \
    _port(other._port), _IP(other._IP), _serv_name(other._serv_name), \
    _sockfd(other._sockfd), _sock_err(other._sock_err), _result(other._result){
    other._result = nullptr;
    other._sockfd = nullptr;
    // _event = other._event;
}

VirtualHost& VirtualHost::operator=(VirtualHost&& other) noexcept {
    if (this != &other){
        if (_result)
            freeaddrinfo(_result);
        _result = other._result;
        other._result = nullptr;
        if (*_sockfd){
            close(*_sockfd);
            *_sockfd = -1;
        }
        _sockfd = other._sockfd;
        other._sockfd = nullptr;

        _info = other._info;
        _port = other._port;
        _IP = other._IP;
        _serv_name = other._serv_name;
        _sock_err = other._sock_err;
        // _event = other._event;
    }
    return (*this);
}

VirtualHost::~VirtualHost(){}

struct addrinfo* VirtualHost::getRes() const{
    return(_result);
}

struct sockaddr* VirtualHost::getAddress() const{
    return(_result->ai_addr);
}

socklen_t VirtualHost::getAddressLength() const{
    return(_result->ai_addrlen);
}

std::string VirtualHost::getIP(void) const{
    return (_IP);
}

std::string VirtualHost::getPort(void) const{
    return (_port);
}

std::string VirtualHost::getServName(void) const{
    return (_serv_name);
}

void    VirtualHost::setup_fd(int* fd){
    _sockfd = fd;
}

int VirtualHost::getFD(void) const{
    return (*_sockfd);
}

void VirtualHost::freeAddress(void){
    freeaddrinfo(_result);
}
/*after this for listening sockets and clients*/

// VirtualHost::VirtualHost(int list_sock_fd){
//     std::cout << "Creating client server//New connection from a client accepted\n";
//     _type = CLIENT;
//     _sockfd = -1;
//     _addr_size = sizeof(_address);
//     if ((_sockfd = accept(list_sock_fd, (struct sockaddr *)&_address, &_addr_size)) == -1){
//         std::cerr << "Error: accept() failed; could not accept client\n";
//         std::cerr << strerror(errno) << "\n";
//         this->~VirtualHost();  //can i do this? 
//     }

//     if (fcntl(_sockfd, F_SETFL, O_NONBLOCK) == -1){
//         std::cerr << "Error: difailed to manipulate client flags\n";
//         std::cerr << strerror(errno) << "\n";
//         this->~VirtualHost();          
//     }
    //get info if you want but not needed 
    // _event.data.fd = _sockfd;
    // _event.events = EPOLLIN | EPOLLET;
    //add to epoll array
    //create connection object with client information
// }

// VirtualHost::VirtualHost(){ //arg is going to change
//     std::cout << "Creating listening socket\n";
//     _type = LISTENING;
//     _sockfd = -1;
//     memset(&_address, 0, sizeof(_address)); //clear out just in case; do we need the 2nd memset then?
//     _address.sin_port = htons(PORT);
//     _address.sin_family = AF_INET;
//     memset(&(_address.sin_zero), '\0', 8); //zero the rest of the struct
//     _addr_size = sizeof(_address);
//     // get the IP address; IP is std::string   NOT ALLOWED?

    
//     if (inet_pton(AF_INET, IP, &(_address.sin_addr.s_addr)) <= 0){
//         std::cerr << "Error: inet_pton() failed\n";
//         std::cerr << strerror(errno) << "\n";
//         return (-1);
//     }

//     if (this->setup_fd() == -1)
//         this->~VirtualHost(); //can I do this?
    
//     _event.data.fd = _sockfd;
//     _event.events = EPOLLIN | EPOLLET;
    
//     //add to epoll array
// }

// VirtualHost::~VirtualHost(){
//     std::cout << "Virtual server destroyed\n";

// }

// int VirtualHost::setup_fd(void){

//     //socket()
//     if (_sockfd = socket(PF_INET, SOCK_STREAM, 0) == -1){
//         std::cerr << "Error: socket() failed\n";
//         std::cerr << strerror(errno) << "\n";
//         return (-1);
//     }
//     //setsockopt: manipulate options for the socket 
//     //CONSIDER: SO_RCVBUF / SO_SNDBUF, SO_LINGER, SO_KEEPALIVE, TCP_NODELAY
//     //get socket error
//     if ((setsockopt(_sockfd, SOL_SOCKET, SO_ERROR, &_sock_err, sizeof(_sock_err))) == -1){
//         std::cerr << "Error: setsockopt() failed: SO_ERROR\n";
//         std::cerr << strerror(errno) << "\n";
//         return (-1);
//     }
//     //reuse port, multiple bind on the same port
//     int reuse_port = 1;
//     if ((setsockopt(_sockfd, SOL_SOCKET, SO_REUSEPORT, &reuse_port, sizeof(reuse_port))) == -1){
//         std::cerr << "Error: setsockopt() failed: SO_REUSEPORT\n";
//         std::cerr << strerror(errno) << "\n";
//         return (-1);
//     }
//     //reuse address: reuse a local address or port that is in the TIME_WAIT state (e.g., after closing a socket
//     int reuse_addr = 1;
//     if ((setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr))) == -1){
//         std::cerr << "Error: setsockopt() failed: SO_REUSEADDR\n";
//         std::cerr << strerror(errno) << "\n";
//         return (-1);
//     }
//     //make it non-blocking
//     if ((fcntl(_sockfd, F_SETFL, O_NONBLOCK)) == -1){
//         std::cerr << "Error: fcntl() failed\n";
//         std::cerr << strerror(errno) << "\n";
//         return (-1);
//     }
//     //for the listening socket bind and listen
//     if ((bind(_sockfd, (struct sockaddr *)&_address, sizeof(struct sockaddr)) == -1)){
//         std::cerr << "Error: bind() failed\n";
//         std::cerr << strerror(errno) << "\n";
//         return (-1);        
//     }
//     if ((listen(_sockfd, 20) == -1)){
//         std::cerr << "Error: listen() failed\n";
//         std::cerr << strerror(errno) << "\n";
//         return (-1);        
//     }
//     return (0);
// }

// int VirtualHost::get_type(void){
//     return(_type);
// }
