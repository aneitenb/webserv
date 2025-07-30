// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<Client.cpp>> -- <<Aida, Ilmari, Milica>>
#include <arpa/inet.h>

#include "utils/message.hpp"
#include "server/Client.hpp"

Client::Client(std::unordered_map<std::string, ServerBlock*> cur, i32 &efd): _allServerNames(cur), _clFd(-1), _count(0), _CGIHandler(this, _clFd, efd), _timeout(CLIENT_DEFAULT_TIMEOUT) {
    _result = nullptr;
    setState(TOADD);
}

Client::~Client(){
    if (_clFd >= 0){
        // close (_clFd);   //CHANGED:don't close if sstill in epoll, eventloop will handle proper cleanup
        _clFd = -1;
    }
    this->_CGIHandler.resolveClose();
}

Client::Client(Client&& other) noexcept : _clFd(-1), _requesting(std::move(other._requesting)), \
    _responding(std::move(other._responding)), _CGIHandler(std::move(other._CGIHandler)){
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
    this->_disconnectAt = other._disconnectAt;
    this->_timeout = other._timeout;
    this->setState(other.getState());
    this->_CGIHandler.setClient(this);
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
        this->_disconnectAt = other._disconnectAt;
        this->_timeout = other._timeout;
        this->setState(other.getState());
        _requesting = std::move(other._requesting);
        _responding = std::move(other._responding);
        this->_CGIHandler = std::move(other._CGIHandler);
        this->_CGIHandler.setClient(this);
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
        && _result == other._result && this->getState() == other.getState())
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

std::string Client::getLocalConnectionIP() {
    struct sockaddr_in localAddr;   //structure that holds IPv4 address information
    socklen_t addrLen = sizeof(localAddr);
    
    // getsockname() is a system call that gets the socket's local address
    // _clFd is the client socket file descriptor
    // (struct sockaddr*)&localAddr is where to store the answer (with casting for compatibility)
    // returns -1 if something goes wrong
    if (getsockname(_clFd, (struct sockaddr*)&localAddr, &addrLen) == -1) {
        Warn("Client::getLocalConnectionIP(): getsockname(" << _clFd
             << "&localAddr, &addrLen) failed: " << strerror(errno));
        return "127.0.0.1"; // fallback
    }
    // inet_ntoa() converts binary IP to human-readable string 
    return std::string(inet_ntoa(localAddr.sin_addr));
}

std::string Client::getLocalConnectionPort() {
    struct sockaddr_in localAddr;
    socklen_t addrLen = sizeof(localAddr);
    
    if (getsockname(_clFd, (struct sockaddr*)&localAddr, &addrLen) == -1) {
        Warn("Client::getLocalConnectionPort(): getsockname(" << _clFd
             << "&localAddr, &addrLen) failed: " << strerror(errno));
        return "8080"; // fallback
    }
    
    return std::to_string(ntohs(localAddr.sin_port));
}

ServerBlock* Client::getSBforResponse(std::string hostHeader){
    // remove port from host header (example.com:8080 -> example.com)
    auto colonPos = hostHeader.find(':');
    std::string serverNameFromHeader = (colonPos != std::string::npos) ? hostHeader.substr(0, colonPos) : hostHeader;

    // get the IP and port the client actually connected to
    std::string connectionIP = getLocalConnectionIP();
    std::string connectionPort = getLocalConnectionPort();

    // try exact match with server_name@connection_ip:connection_port
    std::string exactKey = serverNameFromHeader + "@" + connectionIP + ":" + connectionPort;
    if (_allServerNames.count(exactKey) > 0) {
        return _allServerNames.at(exactKey);
    }
    
    // try match with server_name@wildcard:connection_port
    std::string wildcardKey = serverNameFromHeader + "@0.0.0.0:" + connectionPort;
    if (_allServerNames.count(wildcardKey) > 0) {
        return _allServerNames.at(wildcardKey);
    }
    
    // try connection_ip:connection_port (for empty server names)
    std::string ipPortKey = connectionIP + ":" + connectionPort;
    if (_allServerNames.count(ipPortKey) > 0) {
        return _allServerNames.at(ipPortKey);
    }
    
    // try wildcard IP with port (for empty server names)
    std::string wildcardIpPortKey = "0.0.0.0:" + connectionPort;
    if (_allServerNames.count(wildcardIpPortKey) > 0) {
        return _allServerNames.at(wildcardIpPortKey);
    }
    
    // try to match based on the server_name part only (fallback)
    for (const auto& pair : _allServerNames) {
        std::string key = pair.first;
        size_t atPos = key.find('@');
        
        if (atPos != std::string::npos) {
            std::string keyServerName = key.substr(0, atPos);
            if (keyServerName == serverNameFromHeader) {
                return pair.second;
            }
        }
    }
    
    // fall back to default (first server for this listener)
    return _allServerNames.at(_firstKey);
}

void Client::setKey(std::string key){
    _firstKey = key;
}

/* Overriden*/
int* Client::getSocketFd(int flag) {
    (void)flag;
    return(&_clFd);
}

std::vector<EventHandler*> Client::resolveAccept(void) { return {}; }

void Client::resolveClose(){}

struct epoll_event& Client::getCgiEvent(int flag) { 
    (void)flag;
    return (*this->getEvent()); //wont be used
}

int Client::ready2Switch() { return 1; }

EventHandler* Client::getCgi(){
    return dynamic_cast<EventHandler *>(&this->_CGIHandler);
}

bool Client::conditionMet(std::unordered_map<int*, std::vector<EventHandler*>>& _activeFds, int& epollFd){
    //check if the method is post and if the POST body is not empty
    (void)_activeFds;
    (void)epollFd;
    // if (_theCgi.getProgress() == SENDING || _theCgi.getProgress() == RECEIVING)
    //     return 2;
    if (_requesting.getMethod() == "POST" && _requesting.getBody().size() != 0)
        return 0;
    return 1;
}

int Client::handleEvent(uint32_t ev, [[maybe_unused]] i32 &efd){
    if (ev & EPOLLERR || ev & EPOLLHUP){
        this->setState(CLOSE);
        return (-1);
    }
    if (ev & EPOLLIN){
        if (this->getState() == FORCGI)
            return (0);

        int recvResult = receiving_stuff();
        
        if (recvResult == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return (0); //No more data available right now
            this->setState(CLOSE);  //ADDED
            return (-1); // binary data or real errors close the connection
        } else if (recvResult == 0)
            return (0); //No data available, waiting...
        
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
            if (_requesting.getURI().find(".py") != std::string::npos || _requesting.getURI().find(".php") != std::string::npos) {
                std::string hostHeader;

                try {
                    hostHeader = _requesting.getHeader("Host");
                } catch (const Request::FieldNotFoundException& e) {
                    hostHeader = _firstKey; // Use the default server
                }
                this->_CGIHandler.setServerBlock(getSBforResponse(hostHeader));
                this->_responding = Response(&this->_requesting, getSBforResponse(hostHeader));
                if (!this->_CGIHandler.init(this->_requesting)) {
                    this->setState(TOWRITE);
                    return (0);
                }
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
            _responding.clear();
            this->setState(CLOSE);
            return (-1);
        }
        
        if (_responding.isComplete() == true){
            //check if timeout response was just sent
            if (_responding.getStatusCode() == 408) {
                Debug("408 timeout response sent, closing connection");
                setState(CLOSE);
                return 0;
            }
            // Reset for next request
           if (!_requesting.getMethod().empty() && _requesting.isParsed()) {
                _requesting.reset();
                _responding.clear();
                this->setState(TOREAD);
                _count = 0;
            } else {
                warn("Client::handleEvent(): Invalid or empty request, closing connection");
                this->setState(CLOSE);
                return (-1);
            }
            // BELOW would replace above if/else if we don't want to implement keep-alive
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
    if (this->_CGIHandler.getState() == CGIWRITE) {
        _responding.handleCgi(this->_CGIHandler);
        this->_CGIHandler.setState(CGIDONE);
    } 
    buffer = _responding.getFullResponse();
    if (buffer.size() == 0)
        return (-1);
    Debug("\nSending response to client at socket #" << _clFd);
    if (_responding.isComplete() != true){
        ssize_t len = send(_clFd, buffer.c_str() + _responding.getBytes(), buffer.size() - _responding.getBytes(), 0); //buffer + bytesSentSoFar, sizeof remaining bytes, 0
        if (len < 1){
            Warn("Client::sending_stuff(): send(" << _clFd << ", buffer.c_str() + "
                 << _responding.getBytes() << ", " << buffer.size() - _responding.getBytes()
                 << ", 0) failed: " << strerror(errno));
            return (-1);
        }
        else{ // len > 0
            _responding.addToBytesSent(len);
            //clear rawData?
        }
    }
    return (0);
}

int Client::receiving_stuff(){
    if (_clFd < 0)
        return -1;
    // ssize_t len = 0;
    std::string temp_buff;
    temp_buff.resize(4096);

    ssize_t len = recv(_clFd, &temp_buff[0], temp_buff.size(), 0);
    // len = recv(_clFd, &temp_buff[0], temp_buff.size(), 0);
    
    if (len == 0) //Client closed connection
        return (-1);
    else if (len < 0) { 
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return (0); //No more data available
        Warn("Client::receiving_stuff(): recv(" << _clFd << ", &temp_buff[0], "
             << temp_buff.size() << ", 0) failed: " << strerror(errno));
        return (-1); //real error
    } else {  //something was returned
        temp_buff.resize(len);
        if (temp_buff.size() <= _buffer.max_size() - _buffer.size())
            _buffer.append(temp_buff);
        else
            return (-1);    //buffer overflow
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
        return (_requesting.isParsed()) ? 0 : 1;
    } catch(std::exception& e){
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

void	Client::updateDisconnectTime(void) {
    this->_disconnectAt = std::chrono::system_clock::now() + std::chrono::milliseconds(this->_timeout);
}

void	Client::stopCGI(void) {
    this->_CGIHandler.stop();
}

const timestamp	&Client::getDisconnectTime(void) const { return this->_disconnectAt; }

void Client::sendTimeoutResponse() {    
    _responding.clear();
    _responding.setStatusCode(408);
    _responding.setHeader("Content-Type", "text/html");
    _responding.setHeader("Connection", "close");
    _responding.setHeader("Date", getCurrentDate());
    
    std::string errorBody = 
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head><title>408 Request Timeout</title></head>\n"
        "<body>\n"
        "<h1>408 Request Timeout</h1>\n"
        "<p>The server timed out waiting for the request.</p>\n"
        "</body>\n"
        "</html>";
    
    _responding.setBody(errorBody);
    _responding.prepareResponse();
}

std::string Client::getCurrentDate() const {
    char buffer[100];
    time_t now = time(0);
    struct tm* timeinfo = gmtime(&now);
    strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
    return std::string(buffer);
}
