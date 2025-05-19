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

enum Progress{
    SWITCH,
    RECEIVING,
    SENDING,
    DONE,
    ERROR,
    WAIT
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
        std::vector <std::string> _envS;
        toAndFro fromCGI;
        toAndFro toCGI;
        std::size_t _offset;
        Progress _curr;
        std::string _absPath;
        std::string _scriptPath;
        std::string _scriptName;
        std::string _query;
        std::string _rawdata; //for what's received from the script
        
        // std::string getQueryPath(int side);
        int check_paths();
        bool requestComplete();
        CgiHandler(const CgiHandler& other) = delete;
        CgiHandler& operator=(const CgiHandler& other) = delete;
    public:
        CgiHandler(Request* req, Response* res, int* fd);
        CgiHandler();

        CgiHandler( CgiHandler&& other);
        CgiHandler& operator=( CgiHandler&& other);
        bool operator==(const CgiHandler& other) const;

        ~CgiHandler();

        int handleEvent(uint32_t ev) override;
        int* getSocketFd(int flag) override;
        std::vector<EventHandler*> resolveAccept(void) override;
        void resolveClose() override;
        EventHandler* getCgi() override;
        bool conditionMet(std::unordered_map<int*, std::vector<EventHandler*>>& _activeFds, int& epollFd) override;
        int ready2Switch() override;
        struct epoll_event& getCgiEvent(int flag) override;

        bool compareStructs(const CgiHandler& other) const;
        bool cgiDone();
        int setupPipes();
        int setupEnv();
        int forking(std::unordered_map<int*, std::vector<EventHandler*>>& _activeFds, int& epollFd);
        int run();

        // struct epoll_event& getEvent(int flag);
        int* getFromFd();
        int* getToFd();
        Progress getProgress() const;
        void setProgress(Progress value);
        std::string& getRaw();
        void setReqRes(Request *req, Response *res);
};