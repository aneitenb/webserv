#pragma once

#include <sys/epoll.h>

class EventHandler{
    public:
        virtual ~EventHandler();
        virtual int handleEvent(uint32_t ev) = 0;
};
