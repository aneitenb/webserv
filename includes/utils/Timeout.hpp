// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<Timeout.hpp>> -- <<Aida, Ilmari, Milica>>

#pragma once

#include <list>
#include <chrono>

#include "defs.hpp"
#include "server/Client.hpp"

class Timeout {
	private:
		std::list<std::pair<Client &, timestamp>>	_timeouts;

	public:
		Timeout(void);
		Timeout(const Timeout &other) = delete;
		Timeout	&operator=(const Timeout &other) = delete;
		~Timeout(void);

		const std::pair<Client &, timestamp>	&front(void) const;

		bool	empty(void) const;

		void	removeClient(const Client &client);
		void	updateClient(Client &client);
		void	clear(void);
		void	pop(void);
};
