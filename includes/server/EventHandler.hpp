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
// #include "EventLoop.hpp"

enum State {
    WRITING,
    READING,
    CLOSE,
    TOREAD,
    TOWRITE,
    LISTENER,
    TOADD,
    CLOSEADD
}; /*if epollin && towrite
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
    public:
        virtual ~EventHandler(){};
        virtual int handleEvent(uint32_t ev) = 0;
        virtual int* getSocketFd(void) = 0; //add for the client too?
        virtual std::vector<EventHandler*> resolveAccept() = 0;
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
        //might make sense to freeadrinfo after deleting
};
