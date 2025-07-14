// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<Client.cpp>> -- <<Aida, Ilmari, Milica>>

#include "server/Client.hpp"
#include "CommonFunctions.hpp"
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <fcntl.h>
#include "server/CgiHandler.hpp"

/*Orthodox Cannonical Form*/
Client::Client(): /*_listfd(nullptr), */_clFd(-1), _count(0), /*_curR(EMPTY),*/ _theCgi(&_requesting, &_responding, &_clFd){
	_result = nullptr;
    setState(TOADD);
    // ftMemset(&_event, sizeof(_event)); //do I leave this like this?
}

Client::Client(std::unordered_map<std::string, ServerBlock*> cur): _allServerNames(cur), /*_listfd(nullptr),*/ \
    _clFd(-1), _count(0), /*_curR(EMPTY),*/ \
    _theCgi(CgiHandler(&_requesting, &_responding, &_clFd)){
	_result = nullptr;
    setState(TOADD);
    // ftMemset(&_event, sizeof(_event)); //do I leave this like this?
}

Client::~Client(){
    std::cout << "Client destructor called:" << _clFd << std::endl;
    if (_clFd >= 0){
        // close (_clFd);   //CHANGED:don't close if sstill in epoll, eventloop will handle proper cleanup
        _clFd = -1;
    }
}

Client::Client(Client&& other) noexcept : _clFd(-1), _requesting(std::move(other._requesting)), \
    _responding(std::move(other._responding)), _theCgi(std::move(other._theCgi)){
    _allServerNames = other._allServerNames;
    _firstKey = other._firstKey;
    // _relevant = other._relevant;
    // other._relevant = nullptr;
   /* _listfd = other._listfd;
    other._listfd = nullptr;*/
    this->_clFd = other._clFd;
    other._clFd = -1;
    _count = other._count;
    _result = other._result;
    other._result = nullptr;
    /*_curR = other._curR;*/
    this->setState(other.getState());
}

//this should never be used though
Client& Client::operator=(Client&& other) noexcept{
    if (this != &other){
        _allServerNames = other._allServerNames;
        _firstKey = other._firstKey;
        // _relevant = other._relevant;
        // other._relevant = nullptr;
    /* _listfd = other._listfd;
        other._listfd = nullptr;*/
        this->_clFd = other._clFd;
        other._clFd = -1;
        _count = other._count;
        _result = other._result;
        other._result = nullptr;
        /*_curR = other._curR;*/
        this->setState(other.getState());
        _requesting = std::move(other._requesting);
        _responding = std::move(other._responding);
        _theCgi = std::move(other._theCgi);
    }
    return (*this);
}

bool Client::areServBlocksEq(const Client& other) const{
    if (_allServerNames.size() == other._allServerNames.size()){
        for (auto& pair : _allServerNames){
            auto check = other._allServerNames.find(pair.first);
            if (check == other._allServerNames.end())
                break ;
            auto& checkAgainst = other._allServerNames.at(pair.first);
            if (pair.second == checkAgainst)
                return true;
        }
    }
    return false;
}

// add variables; response and request == operators
bool Client::operator==(const Client& other) const{
    if (areServBlocksEq(other) /*&& _listfd == other._listfd */ \
        && _clFd == other._clFd && _count == other._count \
        && _result == other._result && this->getState() == other.getState() \
        && /*_curR == other._curR &&*/ _theCgi == other._theCgi)
        return (true);
    return (false);
}

/* Helpers */
int Client::setFd(int *fd){
    if (this->_clFd != -1){
        close(_clFd);
        _clFd = -1;}
    if (*fd == -1)
        return (-1);
    this->_clFd = *fd;
    *fd = -1;
    return (0);
}

// int Client::copySocketFd(int* fd){
//     if (this->_clFd != -1){
//         close(_clFd);
//         _clFd = -1;}
//     if (*fd == -1)
//         return (-1);
//     this->_clFd = dup(*fd);
//     if (this->_clFd == -1){
//         std::cout << "Error: dup() failed\n"; //exit?
//         return (-1);
//     }
//     close(*fd);
//     *fd = -1;
//     return (0);
// }

/*Getters and Setters*/
// Request& Client::getRequest(){ return (_requesting);}

std::unordered_map<std::string, ServerBlock*> Client::getServerBlocks() const{
    return (_allServerNames);
}

ServerBlock* Client::getSBforResponse(std::string name){
    // remove port from host header if present ("specific.com:8080" -> "specific.com")
    auto colonPos = name.find(':');
    std::string nameNoPort = (colonPos != std::string::npos) ? name.substr(0, colonPos) : name;

    // try exact match with the server_name
    if (_allServerNames.count(nameNoPort) > 0) {
        return _allServerNames.at(nameNoPort);
    }
    
    // If no exact match, try to find by server_name field in ServerBlock
    for (const auto& pair : _allServerNames) {
        if (pair.second->getServerName() == nameNoPort) {
            return pair.second;
        }
    }
    
    // fall back to default (first server for this listener)
    return _allServerNames.at(_firstKey);
}

void Client::setKey(std::string key){
    _firstKey = key;
}

/* Overriden*/
int* Client::getSocketFd(void) {
    return(&_clFd);
}

std::vector<EventHandler*> Client::resolveAccept(void) { return {}; }

void Client::resolveClose(){}

struct epoll_event& Client::getCgiEvent(int flag) { 
    (void)flag;
    return (*this->getEvent()); //wont be used
}

bool Client::ready2Switch() { return false; }

EventHandler* Client::getCgi(){
    if (_theCgi.run() == 1){
        return (nullptr);
    }
    return (dynamic_cast<EventHandler*>(&_theCgi));
}

bool Client::conditionMet(std::unordered_map<int*, std::vector<EventHandler*>>& _activeFds, int& epollFd){
    //check if the method is post and if the POST body is not empty
    (void)_activeFds;
    (void)epollFd;
    if (_requesting.getMethod() == "POST" && _requesting.getBody().size() != 0)
        return true;
    return false;
}

int Client::handleEvent(uint32_t ev){
    if (ev & EPOLLERR || ev & EPOLLHUP){
        std::cout << "Client FD " << _clFd << " got EPOLLERR/EPOLLHUP - closing\n";
        this->setState(CLOSE);
        return (-1);
    }
    if (ev & EPOLLIN){
        if (this->getState() == FORCGI){
            if (_theCgi.cgiDone() == true)
                this->setState(TOWRITE);
            return (0);
        }

        int recvResult = receiving_stuff();
        
        if (recvResult == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return (0); //No more data available right now
            }
            this->setState(CLOSE);  //ADDED
            return (-1); // binary data or real errors close the connection
        }
        else if (recvResult == 0) {
            return (0); //No data available, waiting...
        }
        
        if (_buffer.empty()) {
            return (0);
        }
        
        // process the request using saveRequest()
        int saveResult = saveRequest();
        
        if (saveResult == -1) {
            if (!_requesting.isValid()) {
                Response errorResponse(&_requesting, getSBforResponse(_requesting.getHeader("Host")));
                errorResponse.setStatusCode(_requesting.getErrorCode()); // 400 for invalid request line
                errorResponse.setBody(errorResponse.getErrorPage(_requesting.getErrorCode()));
                errorResponse.setHeader("Content-Type", "text/html");
                errorResponse.prepareResponse();
                _responding = std::move(errorResponse);
                this->setState(TOWRITE);
                _buffer.clear();
                return (0);
            }
            _buffer.clear();
            this->setState(CLOSE);
            return (-1);    // Error occurred
        } else if (saveResult == 1) {
            // don't clear buffer - keep accumulating data
            return (0); // Request incomplete, need more data
        } else {
            // Request complete (saveResult == 0)
            _buffer.clear(); // CRITICAL
            
            // Check for CGI
            if (_requesting.getURI().find(".py") != std::string::npos) {
                this->setState(TOCGI);
                return (0);
            }
        
            // Prepare response
            saveResponse();
            this->setState(TOWRITE);
            return (0);
        }
    }
    if (ev & EPOLLOUT){
        if (sending_stuff() == -1){
            _count++;
            if (_count >= 50){
                _count = 0;
                _responding.clear();
                this->setState(CLOSE); //ADDED
                return (-1);    //close after too many send failures
            }
            return (0); // Try again
        }
        
        if (_responding.isComplete() == true){
            // Reset for next request
           if (!_requesting.getMethod().empty() && _requesting.isParsed()) {
                std::cout << "Valid request (" << _requesting.getMethod() << "), resetting\n";
                _requesting.reset();
                _responding.clear();
                this->setState(TOREAD);
                _count = 0;
            } else {
                std::cout << "Invalid or empty request, closing connection\n";
                this->setState(CLOSE);
                return (-1);
            }
            // BELOW would be replace above if/else if we don't want to implement keep-alive
            // std::cout << "Response sent, closing connection (no keep-alive)\n";
            // this->setState(CLOSE);
            // return (-1);  // Always close after response
        }
        return (0);
    }
    return (0);
}

// Add a method to properly handle connection timeouts:
bool Client::shouldClose() const {
    // Close if we've had too many consecutive errors
    return _count >= 50;
}

/*Handle Event Helpers*/
int Client::sending_stuff(){
    std::string buffer = {0};
    std::cout << "METHOD SENT TO RESPONSE: '" << _requesting.getMethod() << "'" << std::endl;
    buffer = _responding.getFullResponse();
    if (buffer.size() == 0)
        return (-1);
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "BUFFER:    " << buffer << std::endl;
    if (_responding.isComplete() != true){
        ssize_t len = send(_clFd, buffer.c_str() + _responding.getBytes(), buffer.size() - _responding.getBytes(), 0); //buffer + bytesSentSoFar, sizeof remaining bytes, 0
        if (len < 1){
            std::cout << "Error could not send\n";
            return (-1);
        }
        else{ // len > 0
            _responding.addToBytesSent(len);
            std::cout << "some data sent\n";
            //clear rawData?
        }
    }
    return (0);
}

int Client::receiving_stuff(){
    if (_clFd < 0){
        return -1;
    }
    // ssize_t len = 0;
    std::string temp_buff;
    temp_buff.resize(4096);

    std::cout << "Attempting recv on FD: " << _clFd << std::endl;
    ssize_t len = recv(_clFd, &temp_buff[0], temp_buff.size(), 0);
    // len = recv(_clFd, &temp_buff[0], temp_buff.size(), 0);
    
    if (len == 0) { //Client closed connection
        return (-1);
    }
    else if (len < 0) { 
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return (0); //No more data available
        }
        std::cerr << "DEBUG: recv() error: " << strerror(errno) << std::endl;
        return (-1); //real error
    }
    else {  //something was returned
        temp_buff.resize(len);

        if (temp_buff.size() <= _buffer.max_size() - _buffer.size()) {
            _buffer.append(temp_buff);
        } else {
            return (-1);    //buffer overflow
        }
        temp_buff.clear();
    }
    return (len);
}

int Client::saveRequest(){
    try{
        if (_buffer.size() == 0){
            return (0); //Buffer is empty, nothing to process
        }
        
        _requesting.append(_buffer);
        _buffer.clear();  // Clear immediately after processing
        
        // if (!_requesting.isValid()) {        //even on invlaid request, need to generate error response
        //     return (-1);
        // }
        
        //the thing is what if it's a partial request so not everything has been received? it needs to be updated without being marked as wrong
        if (_requesting.isParsed()) {
            return (0);
        } else {
            return (1); //Request incomplete, waiting for more data
        }
    }
    catch(std::exception& e){
        return (-1);
    }
}

void Client::saveResponse(){
    std::string hostHeader;
    
    try {
        hostHeader = _requesting.getHeader("Host");
    } catch (const Request::FieldNotFoundException& e) {
        hostHeader = _firstKey; // Use the default server
    }

    Response curR(&_requesting, getSBforResponse(hostHeader));
    _responding = std::move(curR);
    _responding.prepareResponse();
}

//SIGNAL HANDLING???
// static void sigint_handler(int signo)
// {
//   (void)close(tcp_server_fd);
//   (void)close(tcp_client_fd);
//   sleep(2);
//   printf("Caught sigINT!\n");
//   exit(EXIT_SUCCESS);
// }

// void register_signal_handler(
// int signum,
// void (*handler)(int))
// {
//   if (signal(signum, handler) == SIG_ERR) {
//      printf("Cannot handle signal\n");
//      exit(EXIT_FAILURE);
//   }
// }
