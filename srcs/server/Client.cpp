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

Client::Client(): _listfd(nullptr), _clFd(-1), _curR(EMPTY){
    ftMemset(&_result, sizeof(_result));
    setState(TOADD);
    _lastActive = time(nullptr);
    // ftMemset(&_event, sizeof(_event)); //do I leave this like this?
}

Client::Client(ServerBlock* cur): _relevant(cur), _listfd(nullptr), _clFd(-1), _curR(EMPTY){
    ftMemset(&_result, sizeof(_result));
    setState(TOADD);
    _lastActive = time(nullptr);
    // ftMemset(&_event, sizeof(_event)); //do I leave this like this?
}

Client::~Client(){
    if (_clFd != -1){
        close (_clFd);
        _clFd = -1;
    }
}

int Client::copySocketFd(int* fd){
    if (this->_clFd != -1){
        close(_clFd);
        _clFd = -1;}
    this->_clFd = dup(*fd);
    if (this->_clFd == -1){
        std::cout << "Error: dup() failed\n"; //exit?
        return (-1);
    }
    close(*fd);
    *fd = -1;
    return (0);
}

int* Client::getSocketFd(void) {
    return(&_clFd);
}

Client::Client(Client&& other) noexcept{
    _relevant = other._relevant;
    other._relevant = nullptr;
    _listfd = other._listfd;
    other._listfd = nullptr;
    this->_clFd = -1;
    this->copySocketFd(&other._clFd);
    other._clFd = -1;
    _result = other._result;
    other._result = nullptr;
    _curR = other._curR;
    this->setState(other.getState());
    _requesting = other._requesting;
}

//this should never be used though
Client& Client::operator=(Client&& other) noexcept {
    if (this != &other){
        _relevant = other._relevant;
        other._relevant = nullptr;
        _listfd = other._listfd;
        other._listfd = nullptr;
        this->copySocketFd(&other._clFd);
        _result = other._result;
        other._result = nullptr;
        _curR = other._curR;
        this->setState(other.getState());
    }
    return (*this);
}

//add variables; response and request == operators
bool Client::operator==(const Client& other){
    if (_relevant == other._relevant &&_listfd == other._listfd \
        && _clFd == other._clFd && _result == other._result \
        && this->getState() == other.getState() && _curR == other._curR)
        return (true);
    return (false);
}

ServerBlock* Client::getServerBlock() const{
    return (_relevant);
}

int Client::saveRequest(){
    try{
        Request curR(_buffer);
        //the thing is what if it's a partial request so not everything has been received? it needs to be updated without being marked as wrong
        if (curR.isParsed() == true){
            _requesting = curR;
            return (0);
        }
    }
    catch(std::exception& e){
        return (-1);
    }
    return (-1);
}

void Client::saveResponse(){
    Response curR(_requesting, getServerBlock());
    _responding = std::move(curR);
}

int Client::handleEvent(uint32_t ev){
    if (ev & EPOLLERR || ev & EPOLLHUP){
        return (-1);
    }
    if (ev & EPOLLIN){
        std::cout << "Receiving\n";
        if (receiving_stuff() == -1)
            this->setState(CLOSE);
        //is it complete, check and set
        if (saveRequest() == 0){
            saveResponse(); //since the response will be formed on a complete request, maybe the constructor can call process request right away?
            _buffer.clear(); //or see how it's handled?
            this->setState(TOWRITE); //EPOLLOUT
        }
    }
    if (ev & EPOLLOUT){
        if (sending_stuff() == -1)
            this->setState(CLOSE);
        if (_responding.allSent() == true){
            this->setState(TOREAD); //EPOLLIN
        }
        //if connection::keep-alive switch to epollout
        //if connection::close close socket + cleanup
        // }
        //data to be sent
        //if the whole thing was sent change what the epoll listens for to epollin 
    }
    return (0);
}
//timeout checks

std::vector<EventHandler*> Client::resolveAccept(void) {
    return {};
}

void Client::resolveClose(){}


int Client::sending_stuff(){
    // response class that has totalBytesThatNeed2BSent + bytesSentSoFar
    //for allSent() check per buffer.size() - 1 maybe
    //did I init _bytesSentSoFar and _totalMsgBytes?
    const std::string& buffer = _responding.getRawData();
    while (_responding.allSent() != true){
        ssize_t len = send(_clFd, &buffer + _responding.getBytes(), buffer.size() - _responding.getBytes(), 0); //buffer + bytesSentSoFar, sizeof remaining bytes, 0
        if (len <= 0){
            if (len == 0 || errno == EAGAIN || errno == EWOULDBLOCK) //cant send anymore, wait again
                break ;
            //close connection
            std::cerr << "Could not send data over the connected socket with the fd of " << _clFd << "\n";
            std::cerr << strerror(errno) << "\n";
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
    ssize_t len = 0;
    std::string temp_buff;
    temp_buff.resize(4096);
    // temp_buff.clear(); //maybe I don't need this?
    if (_curR == CLEAR)
        _buffer.clear(); //maybe can't do this if the request is not complete

    while(1){
        len = recv(_clFd, &temp_buff[0], temp_buff.size(), 0); //sizeof(buffer) - 1?
        if (len == -1){ //either means that there is no more data to read or error
            if (errno == EAGAIN || errno == EWOULDBLOCK) //done reading
                break;
            std::cerr << "Could not receive data over the connected socket with the fd of " << _clFd << "\n";
            std::cerr << strerror(errno) << "\n";
            return (-1); //actual error occurred
        }
        else if(len == 0) //means the client closed connection, no need to send
            return (-1);
        else{ // means something was returned
            temp_buff.resize(len);
            std::cout << "What's here  " << temp_buff << std::endl;
            std::cout << "Says here: " << temp_buff.size() << "     " << _buffer.max_size() << "\n";
            if (temp_buff.size() <= _buffer.max_size() - _buffer.size())
                _buffer.append(temp_buff); //append temp to buffer
            temp_buff.clear();
        }
    }
    return (0);
}

// int Client::settingUp(int* fd){
//     socklen_t addr_size = sizeof(struct sockaddr*);
//     //this should maybe be handled by a listener?
//     if ((_clFd = accept(*fd, _result, &addr_size) == -1)){
//         std::cerr << "Error: accept() failed; could not accept client\n";
//         std::cerr << strerror(errno) << "\n";
//         return (-1);
//     }
//     if (fcntl(_clFd, F_SETFL, O_NONBLOCK) == -1){
//         std::cerr << "Error: difailed to manipulate client flags\n";
//         std::cerr << strerror(errno) << "\n";
//         return (-1);        
//     }   
//     return (0);
// }




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
