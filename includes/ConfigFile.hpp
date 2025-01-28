/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigFile.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aneitenb <aneitenb@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/20 12:47:33 by aneitenb          #+#    #+#             */
/*   Updated: 2025/01/28 14:19:09 by aneitenb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Webserv.hpp"

//Type definitions for readability
typedef std::map<std::string, std::string> ServerDirectives;
typedef std::map<std::string, ServerDirectives> ServerConfig;  // path -> directives
typedef std::vector<ServerConfig> ServerConfigs;

class ConfigurationFile {
private:
	std::ifstream		_configFile;
	std::string			_configContent;
	ServerConfig		_tempServer;
	ServerConfigs		_servers;
	std::vector<size_t>	_ports;
	
	// Private member functions
	int	_readFile(void);
	int	_parseConfigFile(void);
	int	_setupDefaultValues(ServerDirectives& directives);
	int	_validatePort(const std::string& port);

	//Delete copy operators to avoid two objects managing a single file
	ConfigurationFile(ConfigurationFile const &rhs);
	ConfigurationFile& operator=(ConfigurationFile const &rhs);
	
public:
	// Exceptions
	class ErrorOpeningConfFile : public std::exception {
		virtual const char* what() const throw();
	};
	
	class ErrorInvalidConfig : public std::exception {
		virtual const char* what() const throw();
	};
	
	class ErrorInvalidPort : public std::exception {
		virtual const char* what() const throw();
	};

	// Constructor and Destructor
	ConfigurationFile(void);
	~ConfigurationFile(void);

	// Public member functions
	void						initializeConfFile(const std::string& filename);
	const ServerConfigs&		getServers(void) const;
	const std::vector<size_t>&	getPorts(void) const;
};
