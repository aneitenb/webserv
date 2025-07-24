// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<Timeout.cpp>> -- <<Aida, Ilmari, Milica>>

#include "utils/message.hpp"
#include "utils/Timeout.hpp"

Timeout::Timeout(void) {}

Timeout::~Timeout(void) {
	this->_timeouts.clear();
}

const std::pair<Client &, timestamp>	&Timeout::front(void) const {
	return this->_timeouts.front();
}

bool	Timeout::empty(void) const {
	return this->_timeouts.empty();
}

void	Timeout::removeClient(const Client &client) {
	std::list<std::pair<Client &, timestamp>>::iterator	i;

	for (i = this->_timeouts.begin(); i != this->_timeouts.end(); ++i)
		if (&i->first == &client)
			break ;
	if (i != this->_timeouts.end())
		this->_timeouts.erase(i);
}

void	Timeout::updateClient(Client &client) {
	std::list<std::pair<Client &, timestamp>>::iterator	i;

	this->removeClient(client);
	client.updateDisconnectTime();
	for (i = this->_timeouts.begin(); i != this->_timeouts.end(); ++i)
		if (i->second > client.getDisconnectTime())
			break ;
	this->_timeouts.insert(i, std::pair<Client &, timestamp>(client, client.getDisconnectTime()));
}

void	Timeout::clear(void) {
	this->_timeouts.clear();
}

void	Timeout::pop(void) {
	this->_timeouts.pop_front();
}
