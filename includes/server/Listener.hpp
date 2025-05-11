// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<Listener.hpp>> -- <<Aida, Ilmari, Milica>>

#pragma once

#include <map>
#include <iostream>
#include <vector>
#include "VirtualHost.hpp"
#include "EventHandler.hpp"
#include "EventLoop.hpp"
#include "Client.hpp"

class Listener : public EventHandler {
    private:
        int         _sockFd; //before every use check?
        // std::vector<VirtualHost> _knownVHs;
        std::string _port;
        std::string _host;
        std::vector<Client> _activeClients;
        ServerBlock _relevant;
    public:
        Listener();
        Listener(std::string _port, std::string _host);
        ~Listener();
        Listener(const Listener& other);
        Listener& operator=(const Listener& other);
        //move constructor and move assignment operator
        // Listener(Listener&& obj) noexcept;
        // Listener& operator=(Listener&& obj) noexcept;

        //getters and setters
        int  setSocketFd(void);
        void setFd(int fd);
        // std::vector<VirtualHost> getHosts(void) const;

        // void addHost(VirtualHost& cur);
        void addServBlock(ServerBlock& cur);
        ServerBlock* getServBlock();
        const std::string& getPort(void) const; 
        void setPort(const std::string& port);
        const std::string& getHost(void) const; 
        void setHost(const std::string& host);

        // int copySocketFd(const int& fd);//dup not needed, should i get rid of it and use a fd wrapper?

        int handleEvent(uint32_t ev) override;
        int* getSocketFd(void) override;
        std::vector<EventHandler*> resolveAccept(void) override;
        void resolveClose() override;
        EventHandler* getCgi() override;
        bool conditionMet(std::unordered_map<int*, std::vector<EventHandler*>>& _activeFds, int& epollFd) override;
        bool ready2Switch() override;
        struct epoll_event& getCgiEvent(int flag) override;

        void addClient(Client& cur);
        const std::vector<Client>& getClients(void) const;
        void delClient(Client* cur);
    };
