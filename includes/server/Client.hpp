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
#include "server/CGIHandler.hpp"

typedef std::chrono::time_point<std::chrono::system_clock>	timestamp;

class Client : public EventHandler {
    private:
        std::unordered_map<std::string, ServerBlock*> _allServerNames;
        std::string         _firstKey;
        int                 _clFd;
        std::string         _buffer;
        Request             _requesting;
        Response            _responding;
		CGIHandler			_CGIHandler;
		timestamp			_disconnectAt;
		u64					_timeout;
		bool				_timedOut;
		bool				_active;

        int sending_stuff();
        int receiving_stuff();
        int saveRequest();
        void saveResponse();

    public:
		const Request	&request = _requesting;

        Client() = delete;
        Client(const std::unordered_map<std::string, ServerBlock*>& cur, i32 &efd);
        ~Client();

		Client(const Client& other) = delete;
		Client& operator=(const Client& other) = delete;

        //move constructor
        Client(Client&& other) noexcept;
        Client& operator=(Client&& other) noexcept;

        bool operator==(const Client& other) const;
        int setFd(int *fd);
        void setKey(std::string key);

        std::unordered_map<std::string, ServerBlock*> getServerBlocks() const;
        ServerBlock* getSBforResponse(std::string name) const;
        bool areServBlocksEq(const Client& other) const;

        int handleEvent(uint32_t ev, i32 &efd) override;
        int* getSocketFd(int flag) override;
        std::vector<EventHandler*> resolveAccept(void) override;
        void resolveClose() override;
        EventHandler* getCgi() override;
        bool conditionMet(std::unordered_map<int*, std::vector<EventHandler*>>& _activeFds, int& epollFd) override;
        int ready2Switch() override;
        struct epoll_event& getCgiEvent(int flag) override;

		std::string	getLocalIP() const;
		std::string	getLocalPort() const;
		std::string	getPeerPort(void) const;
		std::string	getPeerIP(void) const;
		std::string	getFirstKey(void) const;

		const bool	&isActive(void) const;

		const std::string	&getHost(void) const;

		const timestamp	&getDisconnectTime(void) const;

		void	updateDisconnectTime(void);
		void	setTimeout(const u64 ms);
		void	timeout(void);
		void	stopCGI(void);
};
