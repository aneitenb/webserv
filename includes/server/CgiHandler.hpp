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
    int _fd[2];
    struct epoll_event _event;
};

class CgiHandler : public EventHandler{
    private:
        // const std::string& _path; //should be the full path? assuming already checked
        // Client& _client;
        Request& _request;
        int*    _fd;
        pid_t   _pid;
        std::vector<char*> _envp;

        toAndFro fromCGI;
        toAndFro toCGI;

        std::vector<char*> envp;

        CgiHandler(const CgiHandler& other) = delete;
        CgiHandler& operator=(const CgiHandler& other) = delete;

        std::string getQueryPath(int side);
    public:
        CgiHandler(Request& req, int* fd);
        ~CgiHandler();

        int handleEvent(uint32_t ev) override;
        int* getSocketFd(void) override;
        std::vector<EventHandler*> resolveAccept(void) override;
        void resolveClose() override;

        int setupPipes();
        void setupEnv();
        int forking();
        void run();

        struct epoll_event& getEvent(int flag);
        int* getInFd();
        int* getOutFd();

};