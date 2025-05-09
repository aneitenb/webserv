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
Client::Client(): _listfd(nullptr), _clFd(-1), _count(0), _curR(EMPTY), _theCgi(&_requesting, &_responding, &_clFd){
    ftMemset(&_result, sizeof(_result));
    setState(TOADD);
    // ftMemset(&_event, sizeof(_event)); //do I leave this like this?
}

Client::Client(ServerBlock* cur): _relevant(cur), _listfd(nullptr), \
    _clFd(-1), _count(0), _curR(EMPTY), \
    _theCgi(CgiHandler(&_requesting, &_responding, &_clFd)){
    ftMemset(&_result, sizeof(_result));
    setState(TOADD);
    // ftMemset(&_event, sizeof(_event)); //do I leave this like this?
}

Client::~Client(){
    std::cout << "Client destructor called:" << _clFd << std::endl;
    if (_clFd != -1){
        close (_clFd);
        _clFd = -1;
    }
}

Client::Client(Client&& other) noexcept : _clFd(-1), _requesting(other._requesting), \
    _responding(std::move(other._responding)), _theCgi(std::move(other._theCgi)){
    _relevant = other._relevant;
    other._relevant = nullptr;
    _listfd = other._listfd;
    other._listfd = nullptr;
    this->copySocketFd(&other._clFd);
    _count = other._count;
    _result = other._result;
    other._result = nullptr;
    _curR = other._curR;
    this->setState(other.getState());
}

//this should never be used though
Client& Client::operator=(Client&& other) noexcept{
    if (this != &other){
        _relevant = other._relevant;
        other._relevant = nullptr;
        _listfd = other._listfd;
        other._listfd = nullptr;
        this->copySocketFd(&other._clFd); 
        _count = other._count;
        _result = other._result;
        other._result = nullptr;
        _curR = other._curR;
        this->setState(other.getState());
        _theCgi = std::move(other._theCgi);
    }
    return (*this);
}

// add variables; response and request == operators
bool Client::operator==(const Client& other){
    if (_relevant == other._relevant &&_listfd == other._listfd \
        && _clFd == other._clFd && _count == other._count \
        && _result == other._result && this->getState() == other.getState() \
        && _curR == other._curR && _theCgi == other._theCgi
        && _result == other._result && this->getState() == other.getState() \
        && _curR == other._curR && _theCgi == other._theCgi)
        return (true);
    return (false);
}

/* Helpers */
int Client::copySocketFd(int* fd){
    if (this->_clFd != -1){
        close(_clFd);
        _clFd = -1;}
    if (*fd == -1)
        return (-1);
    this->_clFd = dup(*fd);
    if (this->_clFd == -1){
        std::cout << "Error: dup() failed\n"; //exit?
        return (-1);
    }
    close(*fd);
    *fd = -1;
    return (0);
}

/*Getters and Setters*/
// Request& Client::getRequest(){ return (_requesting);}

ServerBlock* Client::getServerBlock() const{
    return (_relevant);
}

/* Overriden*/
int* Client::getSocketFd(void) {
    return(&_clFd);
}

std::vector<EventHandler*> Client::resolveAccept(void) { return {}; }

void Client::resolveClose(){}

struct epoll_event* Client::getCgiEvent() { return {}; }

EventHandler* Client::getCgi(){
    _theCgi.run();
    return (dynamic_cast<EventHandler*>(&_theCgi));
}

bool Client::conditionMet(){
    //check if the method is post and if the POST body is not empty
    if (_requesting.getMethod() == "POST" && _requesting.getBody().size() != 0)
        return true;
    return false;
}

int Client::handleEvent(uint32_t ev){
    if (ev & EPOLLERR || ev & EPOLLHUP){
        return (-1);
    }
    if (ev & EPOLLIN){
        std::cout << "Receiving\n";
        if (receiving_stuff() == -1){
            _count++;
            if (_count == 100)
                this->setState(CLOSE);
            std::cout << "Count: " << _count << std::endl;
        }
        //is it complete, check and set
        if (saveRequest() == 0){
            saveResponse(); //since the response will be formed on a complete request, maybe the constructor can call process request right away?
            _buffer.clear(); //or see how it's handled?
            //if cgi
            //setState(tocgi)
            //count = -1000
            this->setState(TOWRITE); //EPOLLOUT
            _count = 0;
        }
    }
    if (ev & EPOLLOUT){
        if (_count != -1 && sending_stuff() == -1){
            _count++;
            if (_count == 100)
                this->setState(CLOSE);
        }
        if (_responding.isComplete() == true){
            this->setState(TOREAD); //EPOLLIN
            std::cout << "switching sides\n";
            //clear the request and response?
            _responding.clear();
            _count = 0;
        }
        //if connection::keep-alive switch to epollout
        //if connection::close close socket + cleanup
        // }
        //data to be sent
        //if the whole thing was sent change what the epoll listens for to epollin 
    }
    return (0);
}

/*Handle Event Helpers*/
int Client::sending_stuff(){
    std::string buffer = {0};
    buffer = _responding.getFullResponse();
    if (buffer.size() == 0)
        return (-1);
    std::cout << std::endl;
    std::cout << std::endl;
    std::cout << "BUFFER:    " << buffer << std::endl;
    while (_responding.isComplete() != true){
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
    ssize_t len = 0;
    std::string temp_buff;
    temp_buff.resize(4096);
    // temp_buff.clear(); //maybe I don't need this?
    if (_curR == CLEAR)
        _buffer.clear(); //maybe can't do this if the request is not complete

    while(1){
        len = recv(_clFd, &temp_buff[0], temp_buff.size(), 0); //sizeof(buffer) - 1?
        if (len < 1){ //either means that there is no more data to read or error or client closed connection (len == 0)
            return (-1);
        }
        else{ // means something was returned
            temp_buff.resize(len);
            // std::cout << "What's here  " << temp_buff << std::endl;
            // std::cout << "Says here: " << temp_buff.size() << "     " << _buffer.max_size() << "\n";
            if (temp_buff.size() <= _buffer.max_size() - _buffer.size())
                _buffer.append(temp_buff); //append temp to buffer
            temp_buff.clear();
        }
    }
    return (0);
}

int Client::saveRequest(){
    try{
        _requesting.append(_buffer);
        //the thing is what if it's a partial request so not everything has been received? it needs to be updated without being marked as wrong
        if (_requesting.isParsed() == true){
            std::cout << "PARSED\n"; //here check for cgi?
            return (0);
        }
    }
    catch(std::exception& e){
        return (-1);
    }
    return (-1);
}

void Client::saveResponse(){
    Response curR(&_requesting, getServerBlock());
    _responding = std::move(curR);
    //if not cgi do:
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
