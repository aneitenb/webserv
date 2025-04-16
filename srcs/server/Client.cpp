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

Client::Client():_listfd(nullptr), _clFd(-1), _curR(EMPTY){
    ftMemset(&_result, sizeof(_result));
    setState(TOADD);
    // ftMemset(&_event, sizeof(_event)); //do I leave this like this?
}

Client::~Client(){
    if (_clFd != -1){
        close (_clFd);
        _clFd = -1;
    }
}

int Client::copySocketFd(int* fd){
    if (_clFd != -1){
        close(_clFd);
        _clFd = -1;}
    _clFd = dup(*fd);
    if (_clFd == -1){
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

Client::Client(Client&& other) noexcept {
    _listfd = other._listfd;
    other._listfd = nullptr;
    this->copySocketFd(&other._clFd);
    _result = other._result;
    other._result = nullptr;
    _curR = other._curR;
    this->setState(other.getState());
}

//this should never be used though
Client& Client::operator=(Client&& other) noexcept {
    if (this != &other){
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

bool Client::operator==(const Client& other){
    if (_listfd == other._listfd && _clFd == other._clFd \
    && _result == other._result && this->getState() == other.getState() \
    && _curR == other._curR)
        return (true);
    return (false);
}

// State Client::getState(void) const{
//     return(_curS);
// }

// void Client::setState(State newState){
//     _curS = newState;
// }

int Client::handleEvent(uint32_t ev){
    if (ev & EPOLLERR || ev & EPOLLHUP){
        //error and clear?
        this->setState(CLOSE);
    }
    if (ev & EPOLLIN){
        receiving_stuff();
        //is it complete, check and set
        if (_curR == COMPLETE){
            //respond
            //EPOLLOUT //_curS = TOWRITE
            this->setState(TOWRITE);
        }
        // else if(_curR == CLOSE)
            ///handle here?
    }
    if (ev & EPOLLOUT){
        sending_stuff();
        // if (_responding.state == COMPLETE){
            this->setState(TOREAD);
        //if connection::keep-alive switch to epollout
        //if connection::close close socket + cleanup
        // }
        //data to be sent
        //if the whole thing was sent change what the epoll listens for to epollin 
    }
    return (0);
}
//timeout checks

std::vector<EventHandler*> Client::resolveAccept(void) {};


int Client::sending_stuff(){
    // response class that has totalBytesThatNeed2BSent + bytesSentSoFar
    /*
    ssize_t len = send(_clFd, &_buffer + bytesSentSoFar, total - sent, 0); //buffer + bytesSentSoFar, sizeof remaining bytes, 0
    if ( len == EMSGSIZE)
        the message is too long to pass atomically through the underlying protocol, the msg is not transmitted
        return (-1);
        if (len == -1){
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            break ;
        std::cerr << "Could not send data over the connected socket with the fd of " << _clFd << "\n";
        std::cerr << strerror(errno) << "\n";
        return (-1);
    }
    if (totalSent == bytessentsofar)
        switch to EPOLLIN //_curS = TOREAD
        return (0);
    return (1);
    */
   return (0);
}


int Client::receiving_stuff(){
    ssize_t len = 0;
    std::string temp_buff;
    temp_buff.clear(); //maybe I don't need this?
    if (_curR == CLEAR)
        _buffer.clear(); //maybe can't do this if the request is not complete

    while(1){
        len = recv(_clFd, &temp_buff, sizeof(temp_buff), 0); //sizeof(buffer) - 1?
        if (len == -1){ //either means that there is no more data to read or error
            if (errno == EAGAIN || errno == EWOULDBLOCK) //done reading
                break;
            std::cerr << "Could not receive data over the connected socket with the fd of " << _clFd << "\n";
            std::cerr << strerror(errno) << "\n";
            return (-1); //actual error occurred
        }
        else if(len == 0) //means the client closed connection
        {
            // isRequestComplete();
            if (_curR == COMPLETE)
                this->setState(CLOSE); //if the client is no longer connected then no need to respond, right?
            //cleanup? probably fd needs to be closed?
            return (1);
        }
        else{ // means something was returned
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
