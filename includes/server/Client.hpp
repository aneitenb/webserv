// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<Client.hpp>> -- <<Aida, Ilmari, Milica>>

#pragma once

#include <sys/epoll.h>
#include "EventHandler.hpp"
#include "EventLoop.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "ServerBlock.hpp"



// enum State {
//     WRITING,
//     READING,
//     CLOSE,
//     TOREAD,
//     TOWRITE
// };

enum RequestState {
    PARTIAL,
    EMPTY,
    COMPLETE,
    CLEAR
}; //only clear buffer when there is CLEAR marked

class Client : public EventHandler {
    private:
        ServerBlock*        _relevant;
        int*                _listfd; //do i need this
        int                 _clFd;
        struct sockaddr*    _result; //do i need this if when i accept i just take the fd?
        // struct epoll_event  _event;
        // State               _curS;
        std::string         _buffer;
        RequestState        _curR;
        Request             _requesting;
        Response            _responding;
        //size_t? _lastActive;
    public:
        Client(ServerBlock* cur);
        ~Client();
        Client(const Client& other) = delete;
        Client& operator=(const Client& other) = delete;        // int     getFlag(void) const;
        //move constructor
        Client(Client&& other) noexcept;
        Client& operator=(Client&& other) noexcept;
        bool operator==(const Client& other);
        // void    setFlag(int newState);
        // int     settingUp(int* fd);
        // State getState() const;
        // void setState(State newState);
        // int* getClFd(void);
        ServerBlock* getServerBlock() const;

        int copySocketFd(int* fd);
        int sending_stuff();
        int receiving_stuff();
        int saveRequest();
        void saveResponse();
        
        int handleEvent(uint32_t ev) override;
        int* getSocketFd(void) override;
        std::vector<EventHandler*> resolveAccept(void) override;
        void resolveClose() override;
        
        //timeout??
};
