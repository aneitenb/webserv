// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<Listener.cpp>> -- <<Aida, Ilmari, Milica>>

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <netdb.h> //getaddrinfo
#include <cstring> //memset

#include "utils/message.hpp"
#include "utils/Timeout.hpp"
#include "server/Client.hpp"
#include "server/Listener.hpp"

extern Timeout	timeouts;

/*Constructors and Destructor and Operators*/
Listener::Listener() : _sockFd(-1), _result(nullptr){};

Listener::Listener(std::string port, std::string host) : _sockFd(-1), _port(port), _host(host), _result(nullptr) {}

Listener::Listener(Listener&& obj) noexcept{
    _sockFd = obj._sockFd;
    obj._sockFd = -1;
    _port = obj._port;
    _host = obj._host;
    _activeClients = std::move(obj._activeClients);
    _allServerNames = obj._allServerNames;
    _firstKey = obj._firstKey;
    _result = obj._result;
    obj._result = nullptr;
}

Listener& Listener::operator=(Listener&& obj) noexcept{
    if (this != &obj){
        _sockFd = obj._sockFd;
        obj._sockFd = -1;
        _port = obj._port;
        _host = obj._host;
        _activeClients = std::move(obj._activeClients);
        _allServerNames = obj._allServerNames;
        _firstKey = obj._firstKey;
        _result = obj._result;
        obj._result = nullptr;
    }
    return (*this);
}

Listener::~Listener(){
    if (_sockFd != -1){
        close(_sockFd);
        _sockFd = -1;
    }
    freeAddress();
}

/*Getters and Setters*/

void Listener::setFd(int fd){
    _sockFd = fd;
}

void Listener::addServBlock(ServerBlock& cur, std::string name){
    if (_allServerNames.empty() == true)
        _firstKey = name;
    _allServerNames[name] = &cur;
}

std::unordered_map<std::string, ServerBlock*> Listener::getServBlock(){
    return (_allServerNames);
}

const std::string& Listener::getPort(void) const{
    return (_port);
}

struct sockaddr* Listener::getAddress() const{
    return(_result->ai_addr);
}

void Listener::freeAddress(void){
    if (_result){
        freeaddrinfo(_result);
        _result = nullptr;
    }
}

/*Helper functions*/
int Listener::addressInfo(void){
    struct addrinfo hints;
    int status;

    ftMemset(&hints, sizeof(hints));
    hints.ai_family = AF_INET; //IPv4
    hints.ai_socktype = SOCK_STREAM; //TCP
    hints.ai_flags = AI_PASSIVE; //for binding (listening) maybe not needed if we always provide an IP or hostname
    if ((status = getaddrinfo(_host.c_str(), _port.c_str(), &hints, &_result)) != 0){
		Warn("Listener::addressInfo(): getaddrinfo(" << _host << ", " << _port << ", "
			 << "{AF_PASSIVE, AF_INET, SOCK_STREAM, 0, 0, NULL, NULL, NULL}, &_result) failed: "
			 << ((status == EAI_SYSTEM) ? strerror(errno) : gai_strerror(status)));
        return (-1);
    }
    if (_result && _result->ai_family != AF_INET){
		Warn("Listener::addressInfo(): Unexpected error: _result->ai_family not AF_INET");
        return (-1);
    }
	Debug("Completed fetching listener address info");
    return (0);
}

int Listener::makeNonBlock(int* fd){
    if ((fcntl(*fd, F_SETFL, O_NONBLOCK)) == -1){
		Warn("Listener::makeNonBlock(" << *fd << "): fcntl(" << *fd << ", F_SETFL, O_NONBLOCK) failed: " << strerror(errno));
        return (-1);
    }
    return (0);
}

int Listener::setuping(int *fd){
    if (makeNonBlock(fd) == -1)
		return (-1);
#ifdef __DEBUG
	i32	n = 1;
	if (setsockopt(*fd, SOL_SOCKET, SO_REUSEADDR, &n, sizeof(i32)) == -1)
		return (-1);
#endif /* __DEBUG */
    if ((bind(*fd, this->getAddress(), sizeof(struct sockaddr)) == -1)){
		Warn("Listener::setuping(" << *fd << "): bind(" << *fd << ", this->_getAddress(), 16) failed: " << strerror(errno));
        return (-1);        
    }
    if ((listen(*fd, 20) == -1)){
		Warn("Listener::setuping(" << *fd << "): listern(" << *fd << ", 20) failed: " << strerror(errno));
        return (-1);   
    }
    return (0);
}

int Listener::setSocketFd(void){
    if (_sockFd != -1){
        close(_sockFd);
        _sockFd = -1;
	}
    if ((_sockFd = socket(PF_INET, SOCK_STREAM, 0)) == -1){
		Warn("Listener::setSocketFd(): socket(PF_INET, SOCK_STREAM, 0) failed: " << strerror(errno));
        return (-1);
    }
    if (this->addressInfo() == -1)
        return (-1);
    if ((setuping(&_sockFd)) == -1)
        return (-1);
    int status = fcntl(_sockFd, F_GETFD); //delete
    if (status == -1)
		Warn("Listener::setSocketFd(): fcntl(" << _sockFd << ", F_GETFD) failed: " << strerror(errno));
    return (0);
}

/*Overriden*/
int* Listener::getSocketFd(int flag){
    (void)flag;
    return(&_sockFd);
}

int Listener::handleEvent(uint32_t ev, i32 &efd){
    if (ev & EPOLLIN){ //accept incoming clients while there are clients to be accepted
		debug("\nNew client connection received");
        int curFd = accept(_sockFd, nullptr, nullptr); //think about taking in the client info for security reasons maybe
        if (curFd == -1){
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return (0); //means there are no more clients that wait to be accepted
			Warn("Listener::handleEvent(): accept(" << _sockFd
				 << ", NULL, NULL) failed: " << strerror(errno));
            return (-1);
        }
        //separate setuping into making it nonblocking
        if (makeNonBlock(&curFd) == -1) //set as nonblocking
            return (-1);
        Client curC(this->getServBlock(), efd);
        curC.setKey(_firstKey);
        if (curC.setFd(&curFd) == -1) //pass the socket into Client
            return (-1);
        _activeClients.push_back(std::move(curC));   //move into stable list
        //delete
        int status = fcntl(*_activeClients.back().getSocketFd(0), F_GETFD);
        if (status == -1)
			Warn("Listener::handleEvent(): fcntl(" << *_activeClients.back().getSocketFd(0)
				 << ", F_GETFD) failed: " << strerror(errno));
        // curFd = -1; //i think this won't keep the fds open haha
        // curEL->addClient(&(_activeClients.at(_activeClients.size() - 1)));
		timeouts.updateClient(_activeClients.back());
    } else {
        //rare but it could happen
        //in the case of err, socket is unusable
        //in the case of hup, socket is hanging
		Error("\nFatal: Listener::handleEvent(): Socket " << ((ev & EPOLLERR) ? "unusable" : "hanging"));
        return (-1); //cleanup
    }
    return (0);
}

std::vector<EventHandler*> Listener::resolveAccept(void) {
    std::vector<EventHandler*> clients;

    clients.reserve(_activeClients.size());

    for (Client& client : _activeClients)
        clients.push_back(&client);
        
    return(clients);
}

void Listener::resolveClose(){
    if (_activeClients.empty())
        return;
    
    // use iterator properly - no mixing with indices
    for (auto it = _activeClients.begin(); it != _activeClients.end(); ){
        if (it->getState() == TOCLOSE || it->getState() == CLOSED){
			Debug("\nClosing client at socket #" << *it->getSocketFd(0));
            // close the file descriptor if it's still valid
            if (*(it->getSocketFd(0)) >= 0) {
                close(*(it->getSocketFd(0)));
                *(it->getSocketFd(0)) = -1;  // Mark as closed
            }
            it->setState(CLOSED);
            it = _activeClients.erase(it);  // erase returns next valid iterator
			timeouts.removeClient(*it);
        } else {
            ++it;
        }
    }
}

EventHandler* Listener::getCgi() { return {}; }

bool Listener::conditionMet([[maybe_unused]] std::unordered_map<int*, std::vector<EventHandler*>>& _activeFds, [[maybe_unused]] int& epollFd) {
    return false; 
}

struct epoll_event& Listener::getCgiEvent(int flag) { 
    (void)flag;
    return (*this->getEvent()); //wont be used
}

int Listener::ready2Switch() { return 1; }
