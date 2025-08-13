// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<EventHandler.hpp>> -- <<Aida, Ilmari, Milica>>

#pragma once

#include <sys/epoll.h>
#include <vector>
#include <unistd.h>
#include <unordered_map>

#include "defs.hpp"

enum State {
    WRITING, //client is currently writing
    READING, //client is currently reading
    CLOSE, //client needs to be closed
    TOREAD, //client needs to be switched to read
    TOWRITE, //client needs to be switched to write
    LISTENER, //socket is a listening socket
    TOADD, //fd needs to be added
    CLOSED, //fd has been closed
    TOCLOSE, //deleted from epoll, not yet from vector
    TOCGI, //is creating and starting the CGI
    FORCGI, //this is client waiting for cgi
	CGIWRITE,
	CGIDONE
}; 

class EventHandler{
    private:
        State      _cur;
        struct epoll_event _event;

    public:
        virtual ~EventHandler(){};
        virtual int ready2Switch() = 0;
        virtual int handleEvent(uint32_t ev, i32 &efd) = 0;
        virtual int* getSocketFd(int flag) = 0; //add for the client too?
        virtual std::vector<EventHandler*> resolveAccept() = 0;
        virtual void resolveClose() = 0;
        virtual EventHandler* getCgi() = 0;
        virtual struct epoll_event& getCgiEvent(int flag) = 0;
        virtual bool conditionMet(std::unordered_map<int*, std::vector<EventHandler*>>& _activeFds, int& epollFd) = 0;
    

        State getState() const{
            return (_cur);
        };

        void setState(State newState){
            _cur = newState;
        };

        void closeFd(int *fd){
            if (*fd != -1){
                close (*fd);
                *fd = -1;
            }
        };

        void initEvent(){
            _event.events = EPOLLIN;
            _event.data.fd  = *(getSocketFd(0));
            _event.data.ptr = static_cast<void*>(this);
        };

        struct epoll_event* getEvent(){
            return (&_event);
        };

        void changeEvent(uint32_t curE){
            _event.events = curE;
        };
};
