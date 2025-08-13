// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<Client.cpp>> -- <<Aida, Ilmari, Milica>>

#include <arpa/inet.h>
#include <sys/socket.h>

#include "utils/message.hpp"
#include "utils/Timeout.hpp"
#include "server/Client.hpp"

extern Timeout	timeouts;

static inline CGILocation	*_findCGILocation(std::map<std::string, LocationBlock> locations, const std::string &URI);

Client::Client(const std::unordered_map<std::string, ServerBlock*>& cur, i32 &efd): _allServerNames(cur), _clFd(-1), _CGIHandler(this, _clFd, efd), _timeout(CLIENT_DEFAULT_TIMEOUT), _timedOut(false), _active(false) {
    setState(TOADD);
}

Client::~Client(){
    if (_clFd >= 0){
        _clFd = -1;
    }
    this->_CGIHandler.resolveClose();
	timeouts.removeClient(*this);
}

Client::Client(Client&& other) noexcept : _clFd(-1), _requesting(std::move(other._requesting)), \
    _responding(std::move(other._responding)), _CGIHandler(std::move(other._CGIHandler)){
    _allServerNames = other._allServerNames;
    _firstKey = other._firstKey;
    this->_clFd = other._clFd;
    other._clFd = -1;
    this->_disconnectAt = other._disconnectAt;
    this->_timeout = other._timeout;
    this->setState(other.getState());
    this->_CGIHandler.setClient(this);
	this->_timedOut = other._timedOut;
	this->_active = other._active;
}

Client& Client::operator=(Client&& other) noexcept{
    if (this != &other){
        _allServerNames = other._allServerNames;
        _firstKey = other._firstKey;
        this->_clFd = other._clFd;
        other._clFd = -1;
        this->_disconnectAt = other._disconnectAt;
        this->_timeout = other._timeout;
        this->setState(other.getState());
        _requesting = std::move(other._requesting);
        _responding = std::move(other._responding);
        this->_CGIHandler = std::move(other._CGIHandler);
        this->_CGIHandler.setClient(this);
	this->_timedOut = other._timedOut;
	this->_active = other._active;
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

bool Client::operator==(const Client& other) const{
    if (areServBlocksEq(other)  \
        && _clFd == other._clFd  && this->getState() == other.getState())
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


std::unordered_map<std::string, ServerBlock*> Client::getServerBlocks() const{
    return (_allServerNames);
}

std::string Client::getLocalIP() const {
    struct sockaddr_in localAddr;   //structure that holds IPv4 address information
    socklen_t addrLen = sizeof(localAddr);
    
    if (getsockname(_clFd, (struct sockaddr*)&localAddr, &addrLen) == -1) {
        Warn("Client::getLocalConnectionIP(): getsockname(" << _clFd
             << "&localAddr, &addrLen) failed: " << strerror(errno));
        return "127.0.0.1"; // fallback
    }
    // inet_ntoa() converts binary IP to human-readable string 
    return std::string(inet_ntoa(localAddr.sin_addr));
}

std::string Client::getLocalPort() const {
    struct sockaddr_in localAddr;
    socklen_t addrLen = sizeof(localAddr);
    
    if (getsockname(_clFd, (struct sockaddr*)&localAddr, &addrLen) == -1) {
        Warn("Client::getLocalConnectionPort(): getsockname(" << _clFd
             << "&localAddr, &addrLen) failed: " << strerror(errno));
        return "8080"; // fallback
    }
    
    return std::to_string(ntohs(localAddr.sin_port));
}

std::string	Client::getPeerPort(void) const {
	struct sockaddr_in	addr;
	socklen_t			addrLen = sizeof(addr);

	if (getpeername(this->_clFd, (struct sockaddr *)&addr, &addrLen) == -1) {
		Warn("Client::getPeerPort(): getpeername(" << this->_clFd
			 << "&addr, &addrlen) failed: " << strerror(errno));
		return "";
	}
	return std::to_string(ntohs(addr.sin_port));
}

std::string	Client::getPeerIP(void) const {
	struct sockaddr_in	addr;
	socklen_t			addrLen = sizeof(addr);

	if (getpeername(this->_clFd, (struct sockaddr *)&addr, &addrLen) == -1) {
		Warn("Client::getPeerIP(): getpeername(" << this->_clFd
			 << "&addr, &addrlen) failed: " << strerror(errno));
		return "";
	}
	return std::string(inet_ntoa(addr.sin_addr));
}

std::string	Client::getFirstKey(void) const{
    return (_firstKey);
}

ServerBlock* Client::getSBforResponse(std::string hostHeader) const {
    // remove port from host header (example.com:8080 -> example.com)
    auto colonPos = hostHeader.find(':');
    std::string serverNameFromHeader = (colonPos != std::string::npos) ? hostHeader.substr(0, colonPos) : hostHeader;

    // get the IP and port the client actually connected to
    std::string connectionIP = getLocalIP();
    std::string connectionPort = getLocalPort();


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
            std::string fullAddressKey = keyServerName + "@" + serverNameFromHeader + ":" + connectionPort;

            if (keyServerName == serverNameFromHeader || _allServerNames.count(fullAddressKey) > 0) {
                return pair.second;
            }
        }
    }

    return nullptr;
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
	CGILocation	*CGILocation;
	ServerBlock	*serverConf;

    if (ev & EPOLLERR || ev & EPOLLHUP){
        this->setState(CLOSE);
        return (-1);
    }
    if (ev & EPOLLIN){
        if (this->getState() == FORCGI)
            return (0);

        int recvResult = receiving_stuff();
        
        if (recvResult == -1) {
            // if (errno == EAGAIN || errno == EWOULDBLOCK)
            //     return (0); //No more data available right now
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
                //no IP/servername:port combination exists
                ServerBlock* temp = getSBforResponse(_requesting.getHeader("Host"));
                if (!temp){
                    _buffer.clear();
                    this->setState(CLOSE);
                    return (0);
                }
                Response errorResponse(&_requesting, temp);
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
			_buffer.clear(); // CRITICAL
			serverConf = this->getSBforResponse(this->getHost());
            if (!serverConf){
                this->setState(CLOSE);
                return (0);                
            }
			this->_responding = Response(&this->_requesting, serverConf);
			try {
				CGILocation = _findCGILocation(serverConf->getLocationBlocks(), this->_requesting.getURI());
			} catch (std::bad_alloc &) {
				this->_requesting.setErrorCode(HTTP_INTERNAL_SERVER_ERROR);
				this->setState(TOWRITE);
				return (0);
			}
			if (CGILocation) {
				this->_CGIHandler.setLocation(CGILocation);
				this->_CGIHandler.setServerBlock(serverConf);
				this->_responding = Response(&this->_requesting, serverConf);
				if (!this->_CGIHandler.init(this->_requesting)) {
					this->setState(TOWRITE);
					return (0);
				}
				this->setState(TOCGI);
				return (0);
			}
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
			this->_active = false;
			if (this->_responding.getStatusCode() == HTTP_REQUEST_TIMEOUT) {
				this->setState(CLOSE);
				return (-1);
			}
            // Reset for next request
			if (!_requesting.getMethod().empty() && _requesting.isParsed()) {
                _requesting.reset();
                _responding.clear();
                this->setState(TOREAD);
            } else {
                warn("Client::handleEvent(): Invalid or empty request, closing connection");
                this->setState(CLOSE);
                return (-1);
            }
        }
        return (0);
    }
    return (0);
}

/*Handle Event Helpers*/
int Client::sending_stuff(){
    std::string buffer = {0};

	if (this->_timedOut) {
        //should not happen but just in case
        ServerBlock* temp = getSBforResponse(this->getHost());
        if (!temp)
            temp = getSBforResponse(this->getFirstKey());
		this->_responding = Response(&this->_requesting, temp);
		this->_responding.errorResponse(HTTP_REQUEST_TIMEOUT);
	}
	else if (this->_CGIHandler.getState() == CGIWRITE) {
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
        }
    }
    return (0);
}

int Client::receiving_stuff(){
    if (_clFd < 0)
        return -1;
    std::string temp_buff;
    temp_buff.resize(4096);

    ssize_t len = recv(_clFd, &temp_buff[0], temp_buff.size(), 0);
    
    if (len == 0) //Client closed connection
        return (-1);
    else if (len < 0) { 
        Warn("Client::receiving_stuff(): recv(" << _clFd << ", &temp_buff[0], "
             << temp_buff.size() << ", 0) failed: " << strerror(errno));
        return (-1); //real error
    } else {  //something was returned
		this->_active = true;
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
        
        _requesting.append(*this, _buffer);
        _buffer.clear();  // Clear immediately after processing
        return (_requesting.isParsed()) ? 0 : 1;
    } catch(std::exception& e){
        return (-1);
    }
}

void Client::saveResponse(){
    _responding.prepareResponse();
}

const bool	&Client::isActive(void) const { return this->_active; }

void	Client::updateDisconnectTime(void) {
    this->_disconnectAt = std::chrono::system_clock::now() + std::chrono::milliseconds(this->_timeout);
}

void	Client::setTimeout(const u64 ms) { this->_timeout = ms; }

void	Client::timeout(void) {
	this->_timedOut = true;
	this->_active = false;
}

void	Client::stopCGI(void) {
    this->_CGIHandler.stop();
}

const std::string	&Client::getHost(void) const {
	try {
		return this->_requesting.getHeader("Host");
	} catch (Request::FieldNotFoundException &) {}
	return this->_firstKey;
}

const timestamp	&Client::getDisconnectTime(void) const { return this->_disconnectAt; }

static inline CGILocation	*_findCGILocation(std::map<std::string, LocationBlock> locations, const std::string &URI) {
	std::string	path;

	for (CGILocation location : locations) {
		if (location.second.hasCgiPass()) {
			path = location.first;
			if (URI.find(path, 0) == 0)
				return new CGILocation(location);
		}
	}
	return nullptr;
}
