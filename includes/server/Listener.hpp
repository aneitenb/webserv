// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<Listener.hpp>> -- <<Aida, Ilmari, Milica>>

#pragma once

#include <unordered_map>
#include <iostream>
#include <vector>
#include <arpa/inet.h> //uints, sockaddr_in
#include <string> //std::string
#include <sys/epoll.h> //struct epoll_event
#include <config/ServerBlock.hpp> //to become serverblock
#include <utility> //for std::move
// #include "VirtualHost.hpp"
#include "EventHandler.hpp"
#include "EventLoop.hpp"
#include "Client.hpp"
#include <list>

class Listener : public EventHandler {
    private:
        int         _sockFd; //before every use check?
        // std::vector<VirtualHost> _knownVHs;
        std::string _port;
        std::string _host;
        std::list<Client> _activeClients;   //changed to list for stable memory addresses
        std::unordered_map<std::string, ServerBlock*> _allServerNames;
        std::string _firstKey;
        struct addrinfo*    _result; //needs to be freed freeaddrinfo() but be careful because when copying, pointing to the same address
        // ServerBlock _relevant;
        Listener(const Listener& obj) = delete;
        Listener& operator=(const Listener& obj) = delete;
    public:
        Listener();
        Listener(std::string _port, std::string _host);
        ~Listener();
        Listener(Listener&& obj) noexcept;
        Listener& operator=(Listener&& obj) noexcept;
        //move constructor and move assignment operator
        // Listener(Listener&& obj) noexcept;
        // Listener& operator=(Listener&& obj) noexcept;

        //getters and setters
        int  setSocketFd(void);
        void setFd(int fd);
        // std::vector<VirtualHost> getHosts(void) const;

        // void addHost(VirtualHost& cur);
        void addServBlock(ServerBlock& cur, std::string name);
        std::unordered_map<std::string, ServerBlock*> getServBlock();
        const std::string& getPort(void) const; 
        void setPort(const std::string& port);
        const std::string& getHost(void) const; 
        void setHost(const std::string& host);
        const std::string& getFirstKey() const;
        // void setFirst(std::string key);
        // void addServName(std::string name);

        // int copySocketFd(const int& fd);//dup not needed, should i get rid of it and use a fd wrapper?

        int handleEvent(uint32_t ev) override;
        int* getSocketFd(void) override;
        std::vector<EventHandler*> resolveAccept(void) override;
        void resolveClose() override;
        EventHandler* getCgi() override;
        bool conditionMet(std::unordered_map<int*, std::vector<EventHandler*>>& _activeFds, int& epollFd) override;
        bool ready2Switch() override;
        struct epoll_event& getCgiEvent(int flag) override;

        // void addClient(Client& cur);
        const std::list<Client>& getClients(void) const;    //changed return type to be list instead of vector
        void delClient(Client* cur);

        //from VH
        // void setup_fd(int* fd);
        // // int get_type() const;
        // struct addrinfo* getRes() const;
        struct sockaddr* getAddress() const;
        // socklen_t getAddressLength() const;
        // std::string getIP(void) const;
        // // std::string getPort(void) const;
        // std::string getServName(void) const;
        // int getFD(void) const;
        // // void setIP(const char* IntPro) ;
        // // void setPort(const char* port) ;
        // // void setServName(const char* servName) ;
        int addressInfo(void);
        int setuping(int* fd);
        int makeNonBlock(int* fd);
        void freeAddress(void);
        // void cleanResult(void);
    };
