// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<CgiHandler.hpp>> -- <<Aida, Ilmari, Milica>>

#pragma once

#include "http/Request.hpp"
#include "http/Response.hpp"
#include "EventHandler.hpp"
// #include "Client.hpp"

#define CGI_ROOT "/cgi"
#define CGI_EX "/usr/bin/python3"

struct toAndFro {
    int _fd[2] = {0};
    struct epoll_event _event = { .events = 0, .data = { .u64 = 0 } };
};

class CgiHandler : public EventHandler{
    private:
        // const std::string& _path; //should be the full path? assuming already checked
        // Client& _client;
        Request* _request;
        Response* _response;
        int*    _fd;
        pid_t   _pid;
        std::vector<char*> _envp;
        toAndFro fromCGI;
        toAndFro toCGI;
        bool _isDone;

        std::string getQueryPath(int side);
        CgiHandler(const CgiHandler& other) = delete;
        CgiHandler& operator=(const CgiHandler& other) = delete;
    public:
        CgiHandler(Request* req, Response* res, int* fd);
        CgiHandler();

        CgiHandler( CgiHandler&& other);
        CgiHandler& operator=( CgiHandler&& other);
        bool operator==(const CgiHandler& other);

        ~CgiHandler();

        int handleEvent(uint32_t ev) override;
        int* getSocketFd(void) override;
        std::vector<EventHandler*> resolveAccept(void) override;
        void resolveClose() override;
        EventHandler* getCgi() override;
        bool conditionMet() override;
        struct epoll_event& getCgiEvent(int flag) override;

        bool compareStructs(const CgiHandler& other);
        bool isItDone();
        int setupPipes();
        void setupEnv();
        int forking();
        void run();

        struct epoll_event& getEvent(int flag);
        int* getInFd();
        int* getOutFd();

};