/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServerer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/20 12:47:42 by aneitenb          #+#    #+#             */
/*   Updated: 2025/04/03 15:55:50 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

//STREAM
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

//OTHER
#include <vector>
#include <map>
#include <algorithm>
#include <sys/stat.h>
#include <set>
#include <unistd.h>

#include <exception>

#include "EventLoop.hpp"
#include "Listener.hpp"
#include "VirtualHost.hpp"
#include "config/ServerBlock.hpp"

class	WebServer{
private:
	std::vector<Listener>       _listeners;
	std::vector<VirtualHost>	_virtualHosts;
	std::vector<int>			_fds;
	// EventLoop					_eventLoop;
	WebServer obj(const WebServer& other) = delete;
	WebServer& operator=(const WebServer& other) = delete;
public:
	WebServer();
	~WebServer();

	int	initialize(std::vector<ServerBlock>& serBlocks); //create listening and virtual hosts, set them
	// void	run(void); //epoll + accepting connections + event handling
	void freeStuff(void);
};

// void ftMemset(void *dest, std::size_t count);