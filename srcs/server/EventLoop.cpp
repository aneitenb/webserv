// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<EventLoop.cpp>> -- <<Aida, Ilmari, Milica>>

#include "server/EventLoop.hpp"
#include "EventHandler.hpp"
#include <string.h>
#include <iostream> //cerr
#include <unistd.h> //close

EventLoop::EventLoop() : _epollFd(-1){}

EventLoop::~EventLoop(){
    if (_epollFd != -1){
        (_epollFd);
        _epollFd = -1;
    }
}

//getter for all active fds, key : value; get value based on key
std::vector<EventHandler*> EventLoop::findValue(int *fd){
    return (_activeFds.at(fd));
}

//start the run with init
int EventLoop::run(std::vector<EventHandler*> listFds){

    if (this->startRun() == -1)
        return (-1);
    if (this->addListeners(listFds) == -1)
        return (-1);
    while(1){
        int events2Resolve = epoll_wait(_epollFd, _events, MAX_EVENTS, -1);
        for (int i = 0; i < events2Resolve; i++){
            EventHandler* curE = static_cast<EventHandler*>(_events[i].data.ptr);
            if (curE->handleEvent(_events[i].events) == -1)
                return (-1); //cleanup
            if (curE->getState() == LISTENER){
                std::vector<EventHandler*> curClients = curE->resolveAccept();
                for (int i = 0; i < curClients.size(); i++){
                    if (curClients.at(i)->getState() == TOADD){
                        //add them to the map
                        //add them to the fds
                        //set states to reading
                        //???
                    }
                }
            }
            //loop through fds to see which ones to add and which ones to change
            //get the state and change accordingly like if its towrite say its writing and switch it
        }
    }   
}

//create an epoll instance
int EventLoop::startRun(void){
    //set up
    if ((_epollFd = epoll_create1(0)) == -1){
        std::cerr << "Error: Could not create epoll instance\n";
        strerror(errno);
        return (-1); //won't clean up the other things:/
    }
    return (0);
}

//add listeners to the epoll monitoring
int EventLoop::addListeners(std::vector<EventHandler*> listFds){
    for (std::size_t i = 0; i < listFds.size(); i++){
        struct epoll_event curE;
        curE.events = EPOLLIN;
        curE.data.fd = *listFds.at(i)->getSocketFd();
        curE.data.ptr = static_cast<void*>(&listFds.at(i));

        if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, *listFds.at(i)->getSocketFd(), &curE) == -1){
            std::cerr << "Error: Could not add the file descriptor to the epoll instance\n";
            strerror(errno);
            return (-1);
        }
        listFds.at(i)->setState(LISTENER); //set state to listener
        _activeFds[listFds.at(i)->getSocketFd()]; //add fd to the unordered map 
    }
    return (0);
}

int EventLoop::addToEpoll(int* fd, uint32_t event, EventHandler* object){
    struct epoll_event curE;
    curE.events = event;
    curE.data.fd = *fd;
    curE.data.ptr = static_cast<void*>(object);

    if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, *fd, &curE) == -1){
        std::cerr << "Error: Could not add the file descriptor to the epoll instance\n";
        strerror(errno);
        return (-1);
    }
    // object->setLoop(*this);
    return (0);
}

int EventLoop::modifyEpoll(int* fd, uint32_t event, EventHandler* object){
    struct epoll_event curE;
    curE.events = event;
    curE.data.fd = *fd;
    curE.data.ptr = static_cast<void*>(object);

    if (epoll_ctl(_epollFd, EPOLL_CTL_MOD, *fd, &curE) == -1){
        std::cerr << "Error: Could not modify the file descriptor in the epoll instance\n";
        strerror(errno);
        return (-1);
    }
    return (0);
}

int EventLoop::delEpoll(int* fd, EventHandler* object){
    if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, *fd, 0) == -1){
        std::cerr << "Error: Could not delete the file descriptor from the epoll instance\n";
        strerror(errno);
        return (-1);
    }
    return (0);
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

