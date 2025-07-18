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
#include "log.hpp"
#include "utils/message.hpp"

/*Constructors and Destructor and Operators*/

CgiHandler::CgiHandler() : _request(nullptr), _response(nullptr), _fd(nullptr), _pid(-1), _offset(0), _curr(IDLE){
    this->setState(CGICLOSED);
}  

CgiHandler::CgiHandler(Request* req, Response* res, int* fd) : _request(req), _response(res), _fd(fd), _offset(0), _curr(IDLE){
    fromCGI._fd[0] = -1;
    fromCGI._fd[1] = -1;
    fromCGI._event = {.events = 0, .data = { .u64 = 0 }};
    toCGI._fd[0] = -1;
    toCGI._fd[1] = -1;
    toCGI._event = {.events = 0, .data = { .u64 = 0 }};
    this->setState(CGICLOSED);
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

bool CgiHandler::compareStructs(const CgiHandler& other) const{
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

bool CgiHandler::operator==(const CgiHandler& other) const{
    if (_request && other._request && _response && other._response \
            && _fd && other._fd && *_request == *(other._request) && _response == other._response \
            && _fd == other._fd && _pid == other._pid \
            && _envp == other._envp && _curr == other._curr && _offset == other._offset){
        if (compareStructs(other) == true)
            return true;
        }
    return false;
}

CgiHandler::~CgiHandler(){}

/*Helpers & co.*/

std::string& CgiHandler::getScriptP(){
    return(_scriptPath);
}


int* CgiHandler::getFromFd(){ //notused yet? //CHECK
    return (&fromCGI._fd[0]);
}

int* CgiHandler::getToFd(){ //notused yet? //CHECK
    return (&toCGI._fd[1]);
}

Progress CgiHandler::getProgress() const{
    return (_curr);
}

void CgiHandler::setProgress(Progress value){
    if (value == ERROR){
        error("setProgress: error detected");
    }
    _curr = value;
}

std::string& CgiHandler::getRaw(){ //is it used?
    return (_rawdata);
}

void CgiHandler::setReqRes(Request *req, Response *res){
    _request = req;
    _response = res;
}


int CgiHandler::cgiDone(){
    //INTRODUCE TIMEOUT FOR KILLING THE CHILD
    if (_curr == DONE) //if its error the reponse was set
    {
        _curr = IDLE;
        return (0);
    }
    else if (_curr == ERROR){
        _curr = IDLE;
        return (-1);
    }
    else if (_curr == WAIT){
        int status;
        pid_t result = waitpid(_pid, &status, WNOHANG); //see if the child was terminated
        if (result == _pid){
            _response->handleCgi(_rawdata); //handle it inside
            _curr = IDLE;
            return (0);
        }
    }
    return (1);
}

int CgiHandler::run(){
    if (_request->getMethod() == "POST" && _request->getBody().size() != 0){
        _curr = SENDING;
        this->setState(CGITOREAD);
    }
    else
        _curr = RECEIVING;
    if (setupPipes() == 1)
        return 1;//error
    if (setupEnv() == 1)
        return 1;
    return 0;
    //register to epoll outside then
}

int CgiHandler::setupPipes(){
    if ((pipe(fromCGI._fd) == -1) || (_curr == SENDING && pipe(toCGI._fd) == -1))
        return 1;
    if ((fcntl(fromCGI._fd[0], F_SETFL, O_NONBLOCK) == -1 ) || \
        (_curr == SENDING && fcntl(toCGI._fd[1], F_SETFL, O_NONBLOCK) == -1 ))
        return 1;
    if (_curr == SENDING){
        toCGI._event.events = EPOLLOUT;
        toCGI._event.data.fd = toCGI._fd[1];
        toCGI._event.data.ptr = static_cast<void*>(this);
    }
    fromCGI._event.events = EPOLLIN;
    fromCGI._event.data.fd = fromCGI._fd[0];
    fromCGI._event.data.ptr = static_cast<void*>(this); //this shouldnt be an issue, right?

    return 0;
}

//query: after the ? (without it) or ""
//filename = root + / + uri before ? until /
// std::string CgiHandler::getQueryPath(int side){ //NOT USED?
//     // Request& cur = _client.getRequest();
//     std::string URI = _request->getURI();
//     size_t brPos = URI.find('?');
//     if (brPos != std::string::npos){
//         switch (side){
//             case 1:
//                 return URI.substr(0, brPos);
//             case 2:
//                 return URI.substr(brPos + 1);
//             default:
//                 break;
//         }
//     }
//     return (nullptr);
// }

int CgiHandler::check_paths(){
    std::string uri = _request->getURI();
    if (uri.at(0) == '/')
        uri = uri.substr(1);
    size_t found = uri.find_last_of('/');
    if (found == std::string::npos)
        return 1; //invalid path
    _absPath = uri.substr(0, found);
    if (access(_absPath.c_str(), X_OK) != 0){
        return 1;
    }
    if (_request->getMethod() == "GET"){
        found = uri.find_last_of('?');
        if (found == std::string::npos)
            return 1;
        _scriptPath = '.' + uri.substr(0, found);
        _query = uri.substr(found, uri.size() - found);
        return (0);               
    }
    _scriptPath = uri;
    if (access(_scriptPath.c_str(), X_OK) != 0){
        //script not accessible/readable 500 or 404
        return 1;
    }
    found = _scriptPath.find_last_of('/');
    if (found == std::string::npos)
        _scriptName = _scriptPath;
    else{
        _scriptName = _scriptPath.substr(found + 1, _scriptPath.size() - (found + 1));
    }
    _scriptName = "./" + _scriptName;
    return 0;
}

//envp.data()
int CgiHandler::setupEnv(){

    _envS.clear();
    _envp.clear();
    //check if file can be opened i guess
    if (check_paths() == 1){
        return 1;}
    // envS.push_back("SERVER_PROTOCOL=HTTP/1.1");
    // envS.push_back("GATEWAY_INTERFACE=CGI/1.1");
    _envS.push_back("REQUEST_METHOD=" + _request->getMethod());
    _envS.push_back("SCRIPT_FILENAME=" + _scriptName); //not path but the name of the string from the request?
    _envS.push_back("PATH_INFO=" + _absPath); //see if needs fixing, need path anyway    
    if (_request->getMethod() == "GET")
        _envS.push_back("QUERY_STRING=" + _query);
    if (_request->getMethod() == "POST"){
        _envS.push_back("CONTENT_LENGTH=" + std::to_string(_request->getContentLength()));
        _envS.push_back("CONTENT_TYPE=" + _request->getContentType());
    }

    _envp.reserve(_envS.size() + 1);
    for (size_t i = 0; i < _envS.size(); i++){
        _envp.push_back(const_cast<char*>(_envS.at(i).c_str()));
    }

    _envp.push_back(nullptr);
    return 0;
}

int CgiHandler::forking(std::unordered_map<int*, std::vector<EventHandler*>>& _activeFds, int& epollFd){
    _pid = fork();

    if (_pid == 0){ //child
        char* argv[3] = {0};

        //close everything else, cleanup epoll
        for(auto& pair : _activeFds){
            for (size_t i = 0; i < pair.second.size(); i++){
                closeFd(pair.second.at(i)->getSocketFd(0));
            }
            closeFd(pair.first);
        }

        closeFd(&epollFd);

        if (_curr == SENDING && (dup2(toCGI._fd[0], STDIN_FILENO) == -1)){
            std::cerr << "CHILD: Error: dup2 failed\n";
            return 1;}
        if (dup2(fromCGI._fd[1], STDOUT_FILENO) == -1){
            std::cerr << "CHILD: Error: dup2 failed\n";
            return 1;}

        closeFd(&toCGI._fd[0]);       
        closeFd(&fromCGI._fd[0]);
        closeFd(&toCGI._fd[1]);        
        closeFd(&fromCGI._fd[1]);


        //change into the correct directory
        if (chdir(_absPath.c_str()) != 0)
            return 1;

        std::string temparg = CGI_EX;
        argv[0] = (char *)temparg.c_str();
        argv[1] = (char*)_scriptName.c_str(); //needs fixing
        argv[2] = 0;

        //execve
        // _childLog << "\nargv[2] == " << argv[2] << std::flush;
        execve(temparg.c_str(), const_cast<char* const*>(argv), _envp.data());
        //will continue only if exit
        //set response to error?
        _exit(1); //to avoid flushing parent I/O buffers
    }
    else if (_pid > 0){ //parent
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

bool CgiHandler::requestComplete(){
    int num = 0;
    std::string toFind = "\r\n\r\n";
    std::size_t pos = _rawdata.find(toFind);
    while (pos != std::string::npos){
        num++;
        pos = _rawdata.find(toFind, pos + toFind.length());
    }
    if (num == 2)
        return true;
    return false;
}

int CgiHandler::handleEvent(uint32_t ev){
    std::string buffer = {0};
    if (ev & EPOLLIN){
        buffer.clear();
        buffer.resize(4096);
        ssize_t len = read(fromCGI._fd[0], &buffer[0], buffer.size()); //sizeof(buffer) - 1?
        if (len < 1){ //either means that there is no more data to read or error or client closed connection (len == 0)
            //SAVE response!!!!either error or fullresponse
            if (len == 0){
                if (requestComplete() == true){
                    int status;
                    pid_t result = waitpid(_pid, &status, WNOHANG);
                    if (result != _pid)
                        _curr = WAIT;
                    else
                        _curr = DONE;
                    _response->handleCgi(_rawdata); //handle it inside
                    return (0);
                }
            }
            setProgress(ERROR);
            std::cerr << "Error: delete this after\n";
            std::cerr << strerror(errno) << "\n";
            return (-1);
        }
        else{ // means something was returned
            buffer.resize(len);
            if (buffer.size() <= _rawdata.max_size() - buffer.size())
                _rawdata.append(buffer); //append temp to buffer
            if (requestComplete() == true){
            int status;
            pid_t result = waitpid(_pid, &status, WNOHANG);
            if (result != _pid)
                _curr = WAIT;
            else
                _curr = DONE;
            _response->handleCgi(_rawdata); //handle it inside
            }
        }   
    }
    else if (ev & EPOLLOUT){
        buffer = _request->getBody();
        if (buffer.size() == 0){
            setProgress(ERROR);
            return (0);}
        if (_offset != buffer.size()){
            ssize_t len = write(toCGI._fd[1], buffer.c_str() + _offset, buffer.size() - _offset);
            if (len < 1){
                if (_offset == buffer.size()){
                    _curr = SWITCH;
                    return (0);
                }
                //timeout
                setProgress(ERROR);
                // return (0);
            }
            else{ // len > 0
                _offset += len;
                if (_offset == buffer.size())
                {
                    _curr = SWITCH;
                }
                //clear rawData?
            }
        }
    }
    else
        setProgress(ERROR); //just sets the response
    return 0;
}

int* CgiHandler::getSocketFd(int flag){
    if (flag == 1)
        return (getToFd());
    return (getFromFd());//for SENDING to the original client
}

// struct epoll_event& CgiHandler::getEvent(int flag){ //delete this or the other one?
//     if (flag == 0)
//         return (toCGI._event);
//     return (fromCGI._event);
// }

bool CgiHandler::conditionMet(std::unordered_map<int*, std::vector<EventHandler*>>& _activeFds, int& epollFd) { 
    if (this->forking(_activeFds, epollFd) == 1){
        // this->setProgress(ERROR);
        return 1;}
    return 0;
}

int CgiHandler::ready2Switch() {
    if (_curr == ERROR)
        return (-1);
    if (_curr == SWITCH){
        _curr = RECEIVING;
        return (0);
    }
    if (_curr == SENDING)
        return (2);
    return (1); 
}

std::vector<EventHandler*> CgiHandler::resolveAccept(void){ return {};}

void CgiHandler::resolveClose(){
    closeFd(&fromCGI._fd[0]);
    closeFd(&fromCGI._fd[1]);
    closeFd(&toCGI._fd[0]);
    closeFd(&toCGI._fd[1]);
}

void CgiHandler::setErrorCgi() {}

EventHandler* CgiHandler::getCgi() { return {};}

struct epoll_event& CgiHandler::getCgiEvent(int flag) { 
    if (flag == 1)
        return toCGI._event;
    return fromCGI._event;
}