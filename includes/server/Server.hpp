/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aneitenb <aneitenb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 14:47:13 by aneitenb          #+#    #+#             */
/*   Updated: 2025/03/20 16:05:15 by aneitenb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "../Webserv.hpp"
#include "EventLoop.hpp"
#include "../conf/ConfigFile.hpp"

class	Server{
private:
	std::vector<VirtualServer>	_virtualServer;
	ConfigurationFile			_config;
	EventLoop					_eventLoop;
	
public:
	Server();
	~Server();

	void	initialize();
	void	run();
}
