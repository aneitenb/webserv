// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<Client.hpp>> -- <<Aida, Ilmari, Milica>>

#pragma once

#include <chrono>
#include <sys/epoll.h>

#include "server/EventHandler.hpp"
#include "server/EventLoop.hpp"
#include "http/Request.hpp"
#include "http/Response.hpp"
#include "config/ServerBlock.hpp"
#include "server/CgiHandler.hpp"

typedef std::chrono::time_point<std::chrono::system_clock>	timestamp;

class Client : public EventHandler {
    private:
        std::unordered_map<std::string, ServerBlock*> _allServerNames;
        std::string _firstKey;
        int                 _clFd;
        int                 _count;
        struct sockaddr*    _result; //do i need this if when i accept i just take the fd?
        std::string         _buffer;
        Request             _requesting;
        Response            _responding;
        CgiHandler          _theCgi;
		timestamp			_disconnectAt;
		u64					_timeout;

        int sending_stuff();
        int receiving_stuff();
        int saveRequest();
        void saveResponse();
    public:
        Client();
        Client(std::unordered_map<std::string, ServerBlock*> cur);
        ~Client();

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
        bool shouldClose() const;
        int* getSocketFd(void) override;
        std::vector<EventHandler*> resolveAccept(void) override;
        void resolveClose() override;
        EventHandler* getCgi() override;
        int conditionMet(std::unordered_map<int*, std::vector<EventHandler*>>& _activeFds, int& epollFd) override;
        int ready2Switch() override;
        struct epoll_event& getCgiEvent(int flag) override;
        std::string getLocalConnectionIP(); //new
        std::string getLocalConnectionPort();  //new

		void	updateDisconnectTime(void);

		const timestamp	&getDisconnectTime(void) const;
        
        //timeout??
};
