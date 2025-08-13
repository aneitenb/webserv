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

#include "EventHandler.hpp"
#include "EventLoop.hpp"
#include "Client.hpp"
#include <list>

class Listener : public EventHandler {
    private:
        int         _sockFd;
        std::string _port;
        std::string _host;
        std::string _firstKey;
        struct addrinfo*    _result;

        std::list<Client> _activeClients;
        std::unordered_map<std::string, ServerBlock*> _allServerNames;
        
        Listener(const Listener& obj) = delete;
        Listener& operator=(const Listener& obj) = delete;

    public:
        Listener();
        Listener(std::string _port, std::string _host);
        ~Listener();

        //move constructor and move assignment operator
        Listener(Listener&& obj) noexcept;
        Listener& operator=(Listener&& obj) noexcept;

        //getters and setters
        int  setSocketFd(void);
        void setFd(int fd);

        void addServBlock(ServerBlock& cur, std::string name);
        std::unordered_map<std::string, ServerBlock*> getServBlock();
        const std::string& getPort(void) const; 

        int handleEvent(uint32_t ev, i32 &efd) override;
        int* getSocketFd(int flag) override;
        std::vector<EventHandler*> resolveAccept(void) override;
        void resolveClose() override;
        EventHandler* getCgi() override;
        bool conditionMet(std::unordered_map<int*, std::vector<EventHandler*>>& _activeFds, int& epollFd) override;
        int ready2Switch() override;
        struct epoll_event& getCgiEvent(int flag) override;

        struct sockaddr* getAddress() const;

        int addressInfo(void);
        int setuping(int* fd);
        int makeNonBlock(int* fd);
        void freeAddress(void);
    };
