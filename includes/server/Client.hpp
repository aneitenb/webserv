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
#include "server/EventHandler.hpp"
#include "server/EventLoop.hpp"
#include "http/Request.hpp"
#include "http/Response.hpp"
#include "config/ServerBlock.hpp"
#include "server/CgiHandler.hpp"


// enum State {
//     WRITING,
//     READING,
//     CLOSE,
//     TOREAD,
//     TOWRITE
// };

// enum RequestState {
//     PARTIAL,
//     EMPTY,
//     COMPLETE,
//     CLEAR
// }; //only clear buffer when there is CLEAR marked

class Client : public EventHandler {
    private:
        std::unordered_map<std::string, ServerBlock*> _allServerNames;
        std::string _firstKey;
        // int*                _listfd; //do i need this
        int                 _clFd;
        int                 _count;
        struct sockaddr*    _result; //do i need this if when i accept i just take the fd?
        // struct epoll_event  _event;
        // State               _curS;
        std::string         _buffer;
        // RequestState        _curR;
        Request             _requesting;
        Response            _responding;
        CgiHandler          _theCgi;
        //size_t? _lastActive;

        int sending_stuff();
        int receiving_stuff();
        int saveRequest();
        void saveResponse();
    public:
        Client();
        Client(std::unordered_map<std::string, ServerBlock*> cur);
        ~Client();
        Client(const Client& other) = delete;
        Client& operator=(const Client& other) = delete;        // int     getFlag(void) const;
        //move constructor
        Client(Client&& other) noexcept;
        Client& operator=(Client&& other) noexcept;
        bool operator==(const Client& other) const;
        int setFd(int *fd);
        // void setSockFd(int* fd);
        void setKey(std::string key);
        // void    setFlag(int newState);
        // int     settingUp(int* fd);
        // State getState() const;
        // void setState(State newState);
        // int* getClFd(void);
        std::unordered_map<std::string, ServerBlock*> getServerBlocks() const;
        ServerBlock* getSBforResponse(std::string name);
        bool areServBlocksEq(const Client& other) const;
        // Request& getRequest();
        // Response& getResponse();
        // void setCgi();


        int handleEvent(uint32_t ev) override;
        int* getSocketFd(void) override;
        std::vector<EventHandler*> resolveAccept(void) override;
        void resolveClose() override;
        EventHandler* getCgi() override;
        bool conditionMet(std::unordered_map<int*, std::vector<EventHandler*>>& _activeFds, int& epollFd) override;
        bool ready2Switch() override;
        struct epoll_event& getCgiEvent(int flag) override;
        
        //timeout??
};
