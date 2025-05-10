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
// #include "EventLoop.hpp"

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
    CGI //is cgi
}; 

/*if epollin && towrite
        switch to epollout
        set state to writing
    if epollout && toread
        switch to epollin
        set state to reading
    if close
        close
*/

class EventHandler{
    private:
        // EventLoop* _loop;
        State      _cur;
        struct epoll_event _event;
    public:
        virtual ~EventHandler(){};
        virtual int handleEvent(uint32_t ev) = 0;
        virtual int* getSocketFd(void) = 0; //add for the client too?
        virtual std::vector<EventHandler*> resolveAccept() = 0;
        virtual void resolveClose() = 0;
        virtual EventHandler* getCgi() = 0;
        virtual bool conditionMet(std::unordered_map<int*, std::vector<EventHandler*>>& _activeFds, int& epollFd) = 0;
        virtual struct epoll_event& getCgiEvent(int flag) = 0;
    
        // void setLoop(EventLoop& curLoop){
        //     _loop = &curLoop;
        // };
        // EventLoop& getLoop(void){
        //     return (*_loop);
        // };
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
            _event.data.fd  = *((getSocketFd()));
            _event.data.ptr = static_cast<void*>(this);
        };
        struct epoll_event* getEvent(){
            return (&_event);
        };
        void changeEvent(uint32_t curE){
            _event.events = curE;
        };
        // void setEvent(struct epoll_event* cur, int *fd, uint32_t curE){
        //     cur->events = curE;
        //     cur->data.fd  = *fd;
        //     cur->data.ptr = static_cast<void*>(this);
        // };
        //might make sense to freeadrinfo after deleting
};
