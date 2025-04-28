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
        // std::vector<VirtualHost> getHosts(void) const;

        // void addHost(VirtualHost& cur);
        void addServBlock(ServerBlock& cur);
        ServerBlock* getServBlock();
        const std::string& getPort(void) const; 
        void setPort(const std::string& port);
        const std::string& getHost(void) const; 
        void setHost(const std::string& host);

        int copySocketFd(const int& fd);//dup not needed, should i get rid of it and use a fd wrapper?

        int handleEvent(uint32_t ev) override;
        std::vector<EventHandler*> resolveAccept(void) override;
        int* getSocketFd(void) override;
        void resolveClose() override;

        void addClient(Client& cur);
        std::vector<Client> getClients(void) const;
        void delClient(Client* cur);
    };
