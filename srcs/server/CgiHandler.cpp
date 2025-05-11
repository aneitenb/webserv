// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<CgiHandler.cpp>> -- <<Aida, Ilmari, Milica>>


#include "server/CgiHandler.hpp"
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h> //for exit
#include "http/Request.hpp"
#include "http/Response.hpp"

/*Constructors and Destructor and Operators*/

CgiHandler::CgiHandler() : _request(nullptr), _response(nullptr), _fd(nullptr), _pid(-1), _offset(0), _curr(RECEIVING){}  

CgiHandler::CgiHandler(Request* req, Response* res, int* fd) : _request(req), _response(res), _fd(fd), _offset(0), _curr(RECEIVING){
    fromCGI._fd[0] = -1;
    fromCGI._fd[1] = -1;
    fromCGI._event = {.events = 0, .data = { .u64 = 0 }};
    toCGI._fd[0] = -1;
    toCGI._fd[1] = -1;
    toCGI._event = {.events = 0, .data = { .u64 = 0 }};
}

CgiHandler::CgiHandler(CgiHandler&& other) : _request(other._request), \
    _response(other._response), _fd(other._fd){
        other._request = nullptr;
        other._response = nullptr;
        other._fd = nullptr;
        _pid = other._pid;
        _envp = other._envp;
        fromCGI = other.fromCGI;
        toCGI = other.toCGI;
        _offset = other._offset;
        _curr = other._curr;
}

CgiHandler& CgiHandler::operator=(CgiHandler&& other){
    if (this != &other){
        _request = other._request;
        other._request = nullptr;
        _response = other._response;
        other._response = nullptr;
        _fd = other._fd;
        other._fd = nullptr;
        _pid = other._pid;
        _envp = other._envp;
        fromCGI = other.fromCGI;
        toCGI = other.toCGI;        
        _offset = other._offset;
        _curr = other._curr;
    }
    return (*this);
}

bool CgiHandler::compareStructs(const CgiHandler& other){
    if (fromCGI._event.data.fd == other.fromCGI._event.data.fd \
        && fromCGI._event.data.ptr && other.fromCGI._event.data.ptr \
        && fromCGI._event.data.ptr == other.fromCGI._event.data.ptr \
        && fromCGI._event.data.u32 == other.fromCGI._event.data.u32 \
        && fromCGI._event.data.u64 == other.fromCGI._event.data.u64 \
        && fromCGI._event.events == other.fromCGI._event.events \
        && toCGI._event.data.fd == other.toCGI._event.data.fd \
        && toCGI._event.data.ptr && other.toCGI._event.data.ptr \
        && toCGI._event.data.ptr == other.toCGI._event.data.ptr \
        && toCGI._event.data.u32 == other.toCGI._event.data.u32 \
        && toCGI._event.data.u64 == other.toCGI._event.data.u64 \
        && toCGI._event.events == other.toCGI._event.events)
        return true;
    return false;
}

bool CgiHandler::operator==(const CgiHandler& other){
    if (_request && other._request && _response && other._response \
            && _fd && other._fd && *_request == *(other._request) && _response == other._response \
            && _fd == other._fd && _pid == other._pid && _envp == other._envp \
            && _curr == other._curr && _offset == other._offset){
        if (compareStructs(other) == true)
            return true;
            }
    return false;
}

CgiHandler::~CgiHandler(){}

/*Helpers & co.*/

int* CgiHandler::getInFd(){ //notused yet?
    return (&fromCGI._fd[0]);
}

int* CgiHandler::getOutFd(){ //notused yet?
    return (&toCGI._fd[1]);
}

bool CgiHandler::cgiDone(){
    if (_curr == DONE)
        return true;
    return false;
}

int CgiHandler::run(){
    if (_request->getMethod() == "POST" && _request->getBody().size() != 0)
        _curr = SENDING;
    if (setupPipes() == 1)
        return 1;//error
    if (setupEnv() == 1)
        return 1;
    return 0;
    //register to epoll outside then
}

int CgiHandler::setupPipes(){
    if (pipe(fromCGI._fd) == -1){
        //error
        return 1;
    }
    if (_curr == SENDING && pipe(toCGI._fd) == -1){
        //error
        closeFd (&fromCGI._fd[0]);
        closeFd (&fromCGI._fd[1]);
        return 1;
    }
    if (fcntl(fromCGI._fd[0], F_SETFL, O_NONBLOCK) == -1 ){
        //error
        //massive close?
        closeFd (&toCGI._fd[0]);
        closeFd (&fromCGI._fd[0]);
        closeFd (&toCGI._fd[1]);
        closeFd (&fromCGI._fd[1]);
        return 1;
    }
    if (_curr == SENDING && fcntl(toCGI._fd[1], F_SETFL, O_NONBLOCK) == -1 ){
        closeFd (&toCGI._fd[0]);
        closeFd (&fromCGI._fd[0]);
        closeFd (&toCGI._fd[1]);
        closeFd (&fromCGI._fd[1]);       
        //error
        return 1;
    }
    if (_curr == SENDING){
        toCGI._event.events = EPOLLOUT;
        toCGI._event.data.fd = toCGI._fd[1];
        toCGI._event.data.ptr = static_cast<void*>(this);
    }
    fromCGI._event.events = EPOLLIN;
    fromCGI._event.data.fd = fromCGI._fd[0];
    fromCGI._event.data.ptr = static_cast<void*>(this); //this shouldnt be an issue, right?

    std::cout << "Cgi: Pipe setup complete!\n";
    return 0;
}

//query: after the ? (without it) or ""
//filename = root + / + uri before ? until /
std::string CgiHandler::getQueryPath(int side){ //NOT USED?
    // Request& cur = _client.getRequest();
    std::string URI = _request->getURI();
    size_t brPos = URI.find('?');
    if (brPos != std::string::npos){
        switch (side){
            case 1:
                return URI.substr(0, brPos);
            case 2:
                return URI.substr(brPos + 1);
            default:
                break;
        }
    }
    return (nullptr);
}

int CgiHandler::check_paths(){
    size_t found = _request->getURI().find_last_of('/');
    std::cout << "Cgi: URI: " << _request->getURI() << "\n";
    if (found == std::string::npos)
        return 1; //invalid path
    _absPath = _request->getURI().substr(0, found);
    std::cout << "Cgi: abspath: " << _absPath << "\n";
    if (access(_absPath.c_str(), F_OK | X_OK) != 0){
        //directory not accessible 500 or 404
        return 1;
    }
    found = _request->getURI().find_last_of('?');
    if (found == std::string::npos)
    return 1; //invalid path
    _scriptPath = _request->getURI().substr(0, found);
    std::cout << "Cgi: script: " << _scriptPath << "\n";
    if (access(_scriptPath.c_str(), F_OK | R_OK) != 0){
        //script not accessible/readable 500 or 404
        return 1;
    }
    _query = _request->getURI().substr(found, _request->getURI().size() - found);
    std::cout << "Cgi: query: " << _query << "\n";
    return 0;
}

//envp.data()
int CgiHandler::setupEnv(){
    std::vector <std::string> envS;

    //check if file can be opened i guess
    if (check_paths() == 1)
        return 1;
    _absPath = CGI_ROOT + _absPath;
    envS.push_back("SERVER_PROTOCOL=HTTP/1.1");
    envS.push_back("GATEWAY_INTERFACE=CGI/1.1");
    envS.push_back("REQUEST_METHOD=" + _request->getMethod());
    envS.push_back("QUERY_STRING=" + _query); //ask Ilmari to add + the next one
    envS.push_back("SCRIPT_FILENAME=" + _scriptPath); //not path but the name of the string from the request?
    envS.push_back("PATH_INFO=" + _absPath); //see if needs fixing, need path anyway
    if (_request->getMethod() == "POST"){
        envS.push_back("CONTENT_LENGTH=" + std::to_string(_request->getContentLength()));
        envS.push_back("CONTENT_TYPE=" + _request->getContentType());
    }

    for (size_t i = 0; i < envS.size(); i++){
        _envp.push_back(const_cast<char*>(envS.at(i).c_str()));
    }

    _envp.push_back(NULL);

    std::cout << "Cgi: Environment setup complete!\n";
    return 0;
}

int CgiHandler::forking(std::unordered_map<int*, std::vector<EventHandler*>>& _activeFds, int& epollFd){
    _pid = fork();

    if (_pid == 0){ //child
        char* argv[3] = {0};

        std::cout << "CGI: This is the child!\n";
        //close everything else, cleanup epoll
        for(auto& pair : _activeFds){
            for (size_t i = 0; i < pair.second.size(); i++){
                closeFd(pair.second.at(i)->getSocketFd());
            }
            closeFd(pair.first);
        }

        closeFd(&epollFd);

        closeFd(&fromCGI._fd[0]);
        closeFd(&toCGI._fd[1]);

        if (_curr == SENDING && dup2(toCGI._fd[0], STDIN_FILENO) == -1){
            //error, cleanup
            closeFd(&toCGI._fd[0]);
            closeFd(&fromCGI._fd[1]);
            return 1;
        }
        if (dup2(fromCGI._fd[1], STDIN_FILENO) == -1){
            //error, cleanup
            closeFd(&toCGI._fd[0]);
            closeFd(&fromCGI._fd[1]);
            return 1;
        }
        closeFd(&toCGI._fd[0]);
        closeFd(&fromCGI._fd[1]);

        //change into the correct directory
        if (chdir(_absPath.c_str()) != 0)
            return 1;

        std::string temparg = CGI_EX;
        argv[0] = (char *)temparg.c_str();
        argv[1] = (char*)_absPath.c_str(); //needs fixing
        argv[2] = 0;

        //execve
        execve(temparg.c_str(), const_cast<char* const*>(argv), _envp.data());
        //will continue only if exit
        //set response to error?
        _exit(1); //to avoid flushing parent I/O buffers
    }
    else if (_pid > 0){ //parent
        std::cout << "Cgi: This is the parent!\n";
        closeFd(&fromCGI._fd[1]);
        closeFd(&toCGI._fd[0]);
        /*
        left to do:
        - if it's POST and is not empty: write into toCGI[1]
        - close toCGI[1]
        - read from fromCGI[0]
        - write into clientFd
        - close fromCGI[0]

        int status;
        waitpid(_pid, &status, 0);
        */
    }
    else { //error
        return 1;
    }
    return 0;
}

/*Overriden*/

int CgiHandler::handleEvent(uint32_t ev){
    if (ev & EPOLLERR || ev & EPOLLHUP){
        return (-1);
    }
    std::string buffer = {0};

    if (ev & EPOLLIN){
        buffer.clear();
        buffer.resize(4096);
        std::cout << "Cgi: Receiving\n";
        while(1){
            ssize_t len = recv(fromCGI._fd[0], &buffer[0], buffer.size(), 0); //sizeof(buffer) - 1?
            if (len < 1){ //either means that there is no more data to read or error or client closed connection (len == 0)
                //SAVE response!!!!either error or fullresponse
                if (len == 0){
                    int status;
                    waitpid(_pid, &status, 0);
                    _curr = DONE;
                }
                std::cerr << "Error: delete this after\n";
                std::cerr << strerror(errno) << "\n";
                return (-1);
            }
            else{ // means something was returned
                buffer.resize(len);
                // std::cout << "What's here  " << temp_buff << std::endl;
                // std::cout << "Says here: " << temp_buff.size() << "     " << _buffer.max_size() << "\n";
                if (buffer.size() <= _rawdata.max_size() - buffer.size())
                    _rawdata.append(buffer); //append temp to buffer
                buffer.clear();
            }
        }
    }
    else if (ev & EPOLLOUT){
        std::cout << "Cgi: Sending\n";
        buffer = _request->getBody();
        if (buffer.size() == 0)
            return (-1);
        std::cout << std::endl;
        std::cout << std::endl;
        std::cout << "BUFFER:    " << buffer << std::endl;
        while (_offset != buffer.size()){
            ssize_t len = send(toCGI._fd[1], buffer.c_str() + _offset, buffer.size() - _offset, 0);
            if (len < 1){
                if (_offset == buffer.size())
                    _curr = SWITCH;
                std::cout << "Error could not send" << errno << "\n";
                //timeout
                return (-1);
            }
            else{ // len > 0
                _offset += len;
                std::cout << "some data sent\n";
                //clear rawData?
            }
        }
    }
    return 0;
}

int* CgiHandler::getSocketFd(void){
    if (_curr == SENDING)
        return (getOutFd());
    return (getInFd());//for SENDING to the original client
}

struct epoll_event& CgiHandler::getEvent(int flag){
    if (flag == 0)
        return (fromCGI._event);
    return (toCGI._event);
}

bool CgiHandler::conditionMet(std::unordered_map<int*, std::vector<EventHandler*>>& _activeFds, int& epollFd) { 
    if (this->forking(_activeFds, epollFd) == 1)
        return false;
    return true;
}

bool CgiHandler::ready2Switch() {
    if (_curr == SWITCH){
        _curr = RECEIVING;
        return true;
    }
    return false; 
}

std::vector<EventHandler*> CgiHandler::resolveAccept(void){ return {};}

void CgiHandler::resolveClose(){}

EventHandler* CgiHandler::getCgi() { return {};}

struct epoll_event& CgiHandler::getCgiEvent(int flag) { 
    if (flag == 0)
        return toCGI._event;
    return fromCGI._event;
}