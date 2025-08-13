// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<EventLoop.cpp>> -- <<Aida, Ilmari, Milica>>

#include <csignal>
#include <string.h>
#include <unistd.h> //close

#include "utils/message.hpp"
#include "utils/Timeout.hpp"
#include "server/Client.hpp"
#include "server/EventLoop.hpp"
#include "server/CGIHandler.hpp"
#include "server/EventHandler.hpp"

extern sig_atomic_t gSignal;

static inline i32	_getTimeTo(const timestamp t);

Timeout	timeouts;

EventLoop::EventLoop() : _epollFd(-1){}

EventLoop::~EventLoop(){
    if (_epollFd != -1){
        close(_epollFd);
        _epollFd = -1;
    }
}

int EventLoop::run(std::vector<EventHandler*> listFds){
	EventHandler	*curE;

    if (this->startRun() == -1)
        return (-1);
    this->addListeners(listFds);

	debug("");
	info("Server init done, listening for clients");
    while(gSignal){
        
        //cleanup first
        resolvingClosing();
        
        int events2Resolve = epoll_wait(_epollFd, _events, MAX_EVENTS, (!timeouts.empty()) ? _getTimeTo(timeouts.front().second) : -1);
        
		if (events2Resolve == -1) {
			if (errno != EINTR) {
				Error("Fatal: EventLoop::run(): epoll_wait(" << _epollFd << ", _events, "
						<< MAX_EVENTS << ", -1) failed: " << strerror(errno));
				break ;
			}
			continue ;
		}

		if (events2Resolve == 0) {
			Client &client = timeouts.front().first;
			Debug("\nClient at socket #" << *client.getSocketFd(0) << " timed out, closing connection");
			if (client.isActive() && !client.request.isParsed()) {
				client.setState(TOWRITE);
				resolvingModify(&client, EPOLLOUT);
				client.setTimeout(200);
				client.timeout();
			} else {
				client.stopCGI();
				client.setState(CLOSE);
				timeouts.pop();
			}
            continue ;
		}

		Debug("\nReceived " << events2Resolve << " new event" << ((events2Resolve > 1) ? 's' : '\0'));

        for (int i = 0; i < events2Resolve; i++){
            curE = static_cast<EventHandler*>(_events[i].data.ptr);

            if (!curE) {
				Warn("EventLoop::run(): no event handler found for event #" << i + 1);
                continue; //skip state processing for closed clients
            }

			if (curE == dynamic_cast<Client *>(curE))
				timeouts.updateClient(*dynamic_cast<Client *>(curE));
			else if (curE == dynamic_cast<CGIHandler *>(curE))
				timeouts.updateClient(*dynamic_cast<CGIHandler *>(curE)->getClient());

            if (curE->handleEvent(_events[i].events, this->_epollFd) == -1){
                if (curE->getState() == LISTENER)
                    condemnClients(curE); 
                else
                    curE->setState(CLOSE);
                continue; //skip state processing for closed clients
            }

            State curS = curE->getState();
            switch (curS){
                case LISTENER:
                    resolvingAccept(curE);
                    break;
                case TOWRITE:
                    resolvingModify(curE, EPOLLOUT);
                    break;
                case TOREAD:
                    resolvingModify(curE, EPOLLIN);
                    break;
                case TOCGI:
                    addCGI(static_cast<Client *>(curE));
                    break;
				case CGIWRITE:
					resolvingModify(static_cast<CGIHandler *>(curE)->getClient(), EPOLLOUT);
                default:
                    break;
            }
        }
    }
	timeouts.clear();
    return (0);   
}

void EventLoop::addCGI(Client *client){
	CGIHandler	*CGI;

	CGI = static_cast<CGIHandler *>(client->getCgi());
	if (!CGI->exec(this->_activeFds))
		Warn("EventLoop::addCGI(): Unable to execute CGI: " << strerror(errno));
}

//create an epoll instance
int EventLoop::startRun(void){
    //set up
    if ((_epollFd = epoll_create1(0)) == -1){
		Error("Fatal: EventLoop::startRun(): epoll_create1(0) failed: " << strerror(errno));
        return (-1);
    }
	debug("Epoll instance successfully created");
    return (0);
}

//add listeners to the epoll monitoring
void EventLoop::addListeners(std::vector<EventHandler*> listFds){
    for (std::size_t i = 0; i < listFds.size(); i++){
        listFds.at(i)->initEvent();

        if (*(listFds.at(i)->getSocketFd(0)) == -1 || epoll_ctl(_epollFd, EPOLL_CTL_ADD, *listFds.at(i)->getSocketFd(0), listFds.at(i)->getEvent()) == -1){
			Warn("EventLoop::addListeners(): epoll_ctl(" << _epollFd 
				 << ", EPOLL_CTL_ADD, " << *listFds.at(i)->getSocketFd(0)
				 << ") failed: " << strerror(errno));
            listFds.at(i)->setState(CLOSED);
            listFds.at(i)->closeFd(listFds.at(i)->getSocketFd(0));
            continue;
        }

        listFds.at(i)->setState(LISTENER); //set state to listener
        _activeFds[listFds.at(i)->getSocketFd(0)]; //add fd to the unordered map 
    }
    _listeners = listFds;
}

void EventLoop::condemnClients(EventHandler* cur){
   std::vector<EventHandler*> clients = cur->resolveAccept();

   for (size_t i = 0; i < clients.size(); i++){
        clients.at(i)->setState(CLOSE);
   } 
}

//adding the newly accepted clients to the epoll
void EventLoop::resolvingAccept(EventHandler* cur){
    std::vector<EventHandler*> curClients = cur->resolveAccept();
	Debug("Registering accepted client" << ((curClients.size() > 1) ? 's' : '\0'));
    for (size_t i = 0; i < curClients.size(); i++){
        if (*curClients.at(i)->getSocketFd(0) != -1 && curClients.at(i)->getState() == TOADD){
            if (addToEpoll(curClients.at(i)->getSocketFd(0), curClients.at(i)) == -1){
                curClients.at(i)->setState(CLOSED);
                curClients.at(i)->closeFd(curClients.at(i)->getSocketFd(0));
                continue;}
            curClients.at(i)->setState(READING);
            _activeFds.at(cur->getSocketFd(0)).push_back(curClients.at(i));
			Debug("Registered new client at socket #" << *_activeFds.at(cur->getSocketFd(0)).at(i)->getSocketFd(0));
        }
    }
}

//calling modify on the client
void EventLoop::resolvingModify(EventHandler* cur, uint32_t event){
    if (modifyEpoll(cur->getSocketFd(0), event, cur) == -1)
        cur->setState(CLOSE);
}

EventHandler* EventLoop::getListener(int *fd){
    for (auto& it : _listeners)
        if (fd == it->getSocketFd(0))
            return (it);
    Error("EventLoop::getListener(" << fd << " (" << *fd << ")): Panic: Listener not found in _listeners");
    return {};
}

void EventLoop::resolvingClosing(){
    for (auto& [keyPtr, vect] : _activeFds){
        if (*keyPtr != -1){
            EventHandler* curL = getListener(keyPtr);
            
            if (vect.empty() == true)
                continue;
                
            // CRITICAL FIX: Actually mark clients as TOCLOSE
            bool foundClientsToClose = false;
            for (size_t i = 0; i < vect.size(); i++){
                if (vect.at(i)->getState() == CLOSE){
                    // Remove from epoll BEFORE changing state
                    if (vect.at(i)->getSocketFd(0) && *(vect.at(i)->getSocketFd(0)) >= 0) {
                        delEpoll(vect.at(i)->getSocketFd(0));
                    }
                    // NOW change the state to TOCLOSE
                    vect.at(i)->setState(TOCLOSE);
                    foundClientsToClose = true;
                }
            }
            
            // Only call Listener::resolveClose if we actually found clients to close
            if (foundClientsToClose) {
                curL->resolveClose();
                
                // Update the vector after cleanup
                vect = curL->resolveAccept();
            }
        }
    }
}

//adding a fd to epoll
int EventLoop::addToEpoll(int* fd, EventHandler* object){
    object->initEvent();
    if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, *fd, object->getEvent()) == -1){
		Warn("EventLoop::addToEpoll(" << *fd << ", object): epoll_ctl(" << _epollFd <<
			 ", EPOLL_CTL_ADD, " << *fd << ", object-getEvent()) failed: "
			 << strerror(errno));
        return (-1);
    }
    return (0);
}

//modifying what epoll monitors for a fd
int EventLoop::modifyEpoll(int* fd, uint32_t event, EventHandler* object){
    object->changeEvent(event);

    if (epoll_ctl(_epollFd, EPOLL_CTL_MOD, *fd, object->getEvent()) == -1){
		Warn("EventLoop::modifyEpoll(" << *fd << ", " << event << "object): epoll_ctl("
			 << _epollFd << ", EPOLL_CTL_MOD, " << *fd << ", object-getEvent()) failed: "
			 << strerror(errno));
        return (-1);
    }
    return (0);
}

//removing from the epoll
int EventLoop::delEpoll(int* fd){
    if (!fd || *fd < 0) {
        return 0;
    }
    
    if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, *fd, 0) == -1){
        if (errno == ENOENT){   //FD not found in epoll (ENOENT), this is OK
            return 0;
        }
		Warn("EventLoop::delEpoll(" << *fd << "): epoll_ctl(" << _epollFd
			 << ", EPOLL_CTL_DEL, " << *fd << ", 0) failed: " << strerror(errno));
        return (-1);
    }
    return (0);
}

static inline i32	_getTimeTo(const timestamp t) {
	std::chrono::milliseconds	duration;

	duration = std::chrono::duration_cast<std::chrono::milliseconds>(t - std::chrono::system_clock::now());
	return (duration.count() > 0) ? duration.count() : 0;
}
