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


//from sys/epoll.h
// struct epoll_event {
//     uint32_t      events;  /* Epoll events */
//     epoll_data_t  data;    /* User data variable */
//      or? 
//     void* data.ptr /*can be a ptr to a connection class*/
// };

// union epoll_data {
//     void     *ptr;
//     int       fd;
//     uint32_t  u32;
//     uint64_t  u64;
// };

class Client;

// epoll/select handling
class   EventLoop{
private:
    int                         _epollFd; //for epoll_create1
    // std::vector<int *>            _fds;
    struct epoll_event    _events[MAX_EVENTS]; //result array for epoll_wait()
    // int _maxEvents = MAX_EVENTS;
    // std::vector<Client*> _activeClients;
    std::vector<EventHandler*> _listeners;
    EventLoop(const EventLoop& other) = delete;
    const EventLoop& operator=(const EventLoop& other) = delete;
    std::unordered_map<int*, std::vector<EventHandler*>> _activeFds;
public:
    EventLoop();
    ~EventLoop();
    // void addListenerFds(std::vector<Listener>& listFds);
    int addToEpoll (int* fd, EventHandler* object);
    int modifyEpoll(int* fd, uint32_t event, EventHandler* object);
    int delEpoll(int* fd);
    int startRun();
    void addListeners(std::vector<EventHandler*> listFds);\
    void addCGI(Client *client);
    int run(std::vector<EventHandler*> listFds); //epoll_wait + resolve events: accept/send/recv
    void resolvingAccept(EventHandler* cur);
    void resolvingModify(EventHandler* cur, uint32_t event);
    void resolvingClosing(void);
    void condemnClients(EventHandler* cur);

    //getters, setters
    // std::vector<EventHandler*> findValue(int *fd);
    EventHandler* getListener(int *fd);
};

/*epoll only cares about
- which fd is being monitored
- what kind of event is happening (EPOLLIN, EPOLLOUT...)
- who to notify of the event (handlers)

send only writes 1024 bytes at a time*/
