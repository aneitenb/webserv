// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<VirtualHost.hpp>> -- <<Aida, Ilmari, Milica>>

#pragma once

// #include "config"
#include <arpa/inet.h> //uints, sockaddr_in
#include <string> //std::string
#include <sys/epoll.h> //struct epoll_event
#include <config/ServerBlock.hpp> //to become serverblock
#include <utility> //for std::move
// #include "Listener.hpp"

#define PORT 8080
#define IP "127.0.0.1"

//socket type
#define LISTENING 0
#define CLIENT 1

class VirtualHost {
    private:
        ServerBlock         _info;
        std::string         _port; /*or is all this going to stay parsend in the conif class and we just point at it here?*/
        std::string         _IP;
        std::string         _serv_name;
        int*                 _sockfd;
        // struct sockaddr_in  _address;
        // socklen_t           _addr_size;
        int                 _sock_err; //not needed?
        // int                 _type;
        struct addrinfo*    _result; //needs to be freed freeaddrinfo() but be careful because when copying, pointing to the same address
        // struct epoll_event  _event;
        //locations oor a config file?
    public:
        VirtualHost();
        VirtualHost(const ServerBlock& info, const std::string& port); 
        VirtualHost(const VirtualHost& other);
        VirtualHost& operator=(const VirtualHost& other);
        //move constructor
        VirtualHost(VirtualHost&& other) noexcept;
        VirtualHost& operator=(VirtualHost&& other) noexcept;

        // VirtualHost(); // for listening sockets
        // VirtualHost(int list_sock_fd); //for clients
        ~VirtualHost();
        void setup_fd(int* fd);
        // int get_type() const;
        struct addrinfo* getRes() const;
        struct sockaddr* getAddress() const;
        socklen_t getAddressLength() const;
        std::string getIP(void) const;
        std::string getPort(void) const;
        std::string getServName(void) const;
        int getFD(void) const;
        // void setIP(const char* IntPro) ;
        // void setPort(const char* port) ;
        // void setServName(const char* servName) ;
        int addressInfo(void);
        void freeAddress(void);
};

// struct addrinfo {
//     int              ai_flags;
//     int              ai_family;
//     int              ai_socktype;
//     int              ai_protocol;
//     socklen_t        ai_addrlen;
//     struct sockaddr *ai_addr;
//     char            *ai_canonname;
//     struct addrinfo *ai_next;
// };
