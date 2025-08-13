// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<EventLoop.hpp>> -- <<Aida, Ilmari, Milica>>

#pragma once

#include <vector>
#include <sys/epoll.h>
#include "EventHandler.hpp"

#define MAX_EVENTS 1024

class Client;

class   EventLoop{
    private:
        int _epollFd;
        struct epoll_event          _events[MAX_EVENTS]; //result array for epoll_wait()
        std::vector<EventHandler*>  _listeners;

        EventLoop(const EventLoop& other) = delete;
        const EventLoop& operator=(const EventLoop& other) = delete;

        std::unordered_map<int*, std::vector<EventHandler*>> _activeFds;

    public:
        EventLoop();
        ~EventLoop();

        int addToEpoll (int* fd, EventHandler* object);
        int modifyEpoll(int* fd, uint32_t event, EventHandler* object);
        int delEpoll(int* fd);

        int startRun();
        int run(std::vector<EventHandler*> listFds); //epoll_wait + resolve events: accept/send/recv
        
        void addListeners(std::vector<EventHandler*> listFds);\
        void addCGI(Client *client);

        void resolvingAccept(EventHandler* cur);
        void resolvingModify(EventHandler* cur, uint32_t event);
        void resolvingClosing(void);

        void condemnClients(EventHandler* cur);

        //getters, setters
        EventHandler* getListener(int *fd);
};
