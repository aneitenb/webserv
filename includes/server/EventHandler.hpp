#pragma once

#include <sys/epoll.h>
#include "EventLoop.hpp"

class EventHandler{
    private:
        EventLoop* _loop;
    public:
        virtual ~EventHandler(){};
        virtual int handleEvent(uint32_t ev) = 0;
        void setLoop(EventLoop& curLoop){
            _loop = &curLoop;
        };
        EventLoop& getLoop(void){
            return (*_loop);
        };
        //might make sense to freeadrinfo after deleting
};
