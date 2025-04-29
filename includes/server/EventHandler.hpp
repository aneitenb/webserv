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
    CLOSED //fd has been closed
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
        time_t _lastActive;
    public:
        virtual ~EventHandler(){};
        virtual int handleEvent(uint32_t ev) = 0;
        virtual int* getSocketFd(void) = 0; //add for the client too?
        virtual std::vector<EventHandler*> resolveAccept() = 0;
        virtual void resolveClose() = 0;

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

        void updateTime(){
            _lastActive = time(nullptr);
        };

        const time_t& getLastActive() const{
            return (_lastActive);
        };
        //might make sense to freeadrinfo after deleting
};
