// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<EventLoop.cpp>> -- <<Aida, Ilmari, Milica>>

#include "server/EventLoop.hpp"
#include "server/EventHandler.hpp"
#include <string.h>
#include <iostream> //cerr
#include <unistd.h> //close
#include <csignal>
// #include "CgiHandler.hpp"

extern sig_atomic_t gSignal;

EventLoop::EventLoop() : _epollFd(-1){}

EventLoop::~EventLoop(){
    if (_epollFd != -1){
        close(_epollFd);
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
    this->addListeners(listFds);

    while(gSignal){
        int events2Resolve = epoll_wait(_epollFd, _events, MAX_EVENTS, -1);
        for (int i = 0; i < events2Resolve; i++){
            EventHandler* curE = static_cast<EventHandler*>(_events[i].data.ptr);
            std::cout << "After receiving events\n";
            if (curE->handleEvent(_events[i].events) == -1){
                //error occurred
                if (curE->getState() == LISTENER)
                    condemnClients(curE); 
                else
                    curE->setState(CLOSE);
                continue;
            }
            std::cout << "After handling\n";

            State curS = curE->getState();
            switch (curS){
                case LISTENER:
                    resolvingAccept(curE);
                    break;
                case TOWRITE:
                    resolvingModify(curE, EPOLLOUT);  //handled event was receiving, now it needs to be sending
                    break;
                case TOREAD:
                    resolvingModify(curE, EPOLLIN); //handled event was sending, now it needs to be receiving 
                    break;
                case TOCGI:
                    addCGI(curE);
                    break;
                // case CGI:
                //     handleCGI(curE); //when you send and receive
                //     break;
                default:
                    break;
            }
            std::cout << "After one loop\n";
            curE = nullptr;
        }
        //timeout?
        resolvingClosing();
        std::cout << "After closing\n";
    }
    return (0);   
}

void EventLoop::addCGI(EventHandler* cur){
    EventHandler* theCGI = cur->getCgi();
    if (!theCGI){
        //set response to 500 or 404
        cur->setState(WRITING);
        return ;
    }
    struct epoll_event& curOut = theCGI->getCgiEvent(1);

    theCGI->setState(CGI);
    if (cur->conditionMet(_activeFds, _epollFd) == true){ //pass the activefds so they can close in the child
        struct epoll_event& curIn = theCGI->getCgiEvent(0);
        if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, curIn.data.fd, &curIn) == -1){ //not correct
            //set response
            cur->setState(WRITING);
            return ;
        }
    }
    if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, curOut.data.fd, &curOut) == -1){ //not correct
        //set response
        cur->setState(WRITING);
        return ;
    }
    if (theCGI->conditionMet(_activeFds, _epollFd) == false){
        //set response to 500 or 404
        cur->setState(WRITING);
    }
}

//create an epoll instance
int EventLoop::startRun(void){
    //set up
    if ((_epollFd = epoll_create1(0)) == -1){
        std::cerr << "Error: Could not create epoll instance: ";
        std::cerr << strerror(errno) << "\n";
        return (-1);
    }
    return (0);
}

//add listeners to the epoll monitoring
void EventLoop::addListeners(std::vector<EventHandler*> listFds){
    for (std::size_t i = 0; i < listFds.size(); i++){
        listFds.at(i)->initEvent();

        if (*(listFds.at(i)->getSocketFd()) == -1 || epoll_ctl(_epollFd, EPOLL_CTL_ADD, *listFds.at(i)->getSocketFd(), listFds.at(i)->getEvent()) == -1){
            std::cerr << "Error: Could not add the listening socket to the epoll instance: ";
            std::cerr << strerror(errno) << "\n";
            listFds.at(i)->setState(CLOSED);
            listFds.at(i)->closeFd(listFds.at(i)->getSocketFd());
            continue;
        }

        listFds.at(i)->setState(LISTENER); //set state to listener
        _activeFds[listFds.at(i)->getSocketFd()]; //add fd to the unordered map 
        for (auto& x : _activeFds){
            std::cout << "checking active Fds: " << *(x.first) << std::endl;
        }
    }
    _listeners = listFds;
    for (auto& x : _listeners){
        std::cout << "checking lists: " << *(x->getSocketFd()) << std::endl;
    }
}

void EventLoop::condemnClients(EventHandler* cur){
   std::vector<EventHandler*> clients = findValue(cur->getSocketFd());

   for (size_t i = 0; i < clients.size(); i++){
        clients.at(i)->setState(CLOSE);
   } 
}

//adding the newly accepted clients to the epoll
void EventLoop::resolvingAccept(EventHandler* cur){
    std::vector<EventHandler*> curClients = cur->resolveAccept();
    // if (curClients.empty())
    std::cout << "Trying to add clients\n";
    for (size_t i = 0; i < curClients.size(); i++){
        if (*curClients.at(i)->getSocketFd() != -1 && curClients.at(i)->getState() == TOADD){
            if (addToEpoll(curClients.at(i)->getSocketFd(), curClients.at(i)) == -1){
                curClients.at(i)->setState(CLOSED);
                curClients.at(i)->closeFd(curClients.at(i)->getSocketFd());
                //there shouldn't be anything to clean from Response, Request probably
                continue;}
            curClients.at(i)->setState(READING);
            _activeFds.at(cur->getSocketFd()).push_back(curClients.at(i));
            std::cout << "Client added with fd: " << *(_activeFds.at(cur->getSocketFd()).at(i)->getSocketFd()) << std::endl;
        }
    }
}

//calling modify on the client
void EventLoop::resolvingModify(EventHandler* cur, uint32_t event){
    if (modifyEpoll(cur->getSocketFd(), event, cur) == -1)
        cur->setState(CLOSE);
    std::cout << "Client event changed\n";
}

EventHandler* EventLoop::getListener(int *fd){
    for (auto& it : _listeners)
        if (fd == it->getSocketFd())
            return (it);
    std::cout << "SHOULD NEVER HAPPEN\n";
    return {};
}

//resolve closing and removing of the done/disconnected clients
void EventLoop::resolvingClosing(){
    for (auto& pair : _activeFds){
        if (*pair.first != -1){
            //update vectors
            // std::size_t i = 0;
            EventHandler* curL = getListener(pair.first);
            pair.second = curL->resolveAccept();
            if (pair.second.empty() == true)
                return ;
            for (size_t i = 0; i < pair.second.size(); i++){
                if (pair.second.at(i)->getState() == CLOSE){
                    //cleanup Request, Response, buffer
                    delEpoll(pair.second.at(i)->getSocketFd());
                    pair.second.at(i)->setState(TOCLOSE);
                }
            }
            curL->resolveClose();
            pair.second = curL->resolveAccept();
            std::cout << std::boolalpha;
            std::cout << "Clients empty: " << pair.second.empty() << "\n";
            std::cout << std::noboolalpha;
            std::cout << "Client vector size: " << pair.second.empty() << "\n";
        }
    }
}

//adding a fd to epoll
int EventLoop::addToEpoll(int* fd, EventHandler* object){
    // struct epoll_event curE;
    // curE.events = event;
    // curE.data.fd = *fd;
    // curE.data.ptr = static_cast<void*>(object);
    object->initEvent();
    if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, *fd, object->getEvent()) == -1){
        std::cerr << "Error: Could not add the file descriptor to the epoll instance: ";
        std::cerr << strerror(errno) << "\n";
        return (-1);
    }
    // object->setLoop(*this);
    return (0);
}

//modifying what epoll monitors for a fd
int EventLoop::modifyEpoll(int* fd, uint32_t event, EventHandler* object){
    // struct epoll_event curE;
    // curE.events = event;
    // curE.data.fd = *fd;
    // curE.data.ptr = static_cast<void*>(object);
    object->changeEvent(event);

    if (epoll_ctl(_epollFd, EPOLL_CTL_MOD, *fd, object->getEvent()) == -1){
        std::cerr << "Error: Could not modify the file descriptor in the epoll instance: ";
        std::cerr << strerror(errno) << "\n";
        return (-1);
    }
    return (0);
}

//removing from the epoll
int EventLoop::delEpoll(int* fd){
    if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, *fd, 0) == -1){
        std::cerr << "Error: Could not delete the file descriptor from the epoll instance: ";
        std::cerr << strerror(errno) << "\n";
        return (-1);
    }
    std::cout << *fd << "got deleted from monitoring!\n";
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

