/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigFile.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aneitenb <aneitenb@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/28 09:56:54 by aneitenb          #+#    #+#             */
/*   Updated: 2025/01/29 16:57:08 by aneitenb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ConfigFile.hpp"
#include "../includes/ConfigErrors.hpp"

ConfigurationFile::ConfigurationFile(void) {}

ConfigurationFile::~ConfigurationFile(void) {
	if (_configFile.is_open()) {
		_configFile.close();
	}
}

void ConfigurationFile::initializeConfFile(const std::string& filename) {
	_configFile.open(filename.c_str());
	if (_configFile.fail())
		throw ErrorOpeningConfFile();
	if (_readFile() == -1)
		throw ErrorInvalidConfig("Empty configuration file");
	if (_parseConfigFile() == -1)
		throw ErrorInvalidConfig("invalid configuration format");
}

int ConfigurationFile::_readFile(void) {
	std::stringstream buffer;
	buffer << _configFile.rdbuf();
	_configContent = buffer.str();
	_configFile.close();
	
	if (_configContent.empty())
		return -1;
	return 0;
}

int ConfigurationFile::_parseConfigFile(void) {
	// Basic day-one parsing: Just handle server blocks with port and host
	ServerDirectives directives;
	
	// For day one, we'll just set up a basic server configuration
	_setupDefaultValues(directives);
	
	// Validate the port
	if (_validatePort(directives["listen"]) == -1)
		throw ErrorInvalidPort("Port number must be between 0 and 65535");
		
	// Store the configuration
	_tempServer["/"] = directives;
	_servers.push_back(_tempServer);
	
	return 0;
}

int ConfigurationFile::_setupDefaultValues(ServerDirectives& directives) {
	// Set default values for a basic server configuration
	directives["listen"] = "8080";
	directives["host"] = "127.0.0.1";
	directives["server_name"] = "localhost";
	directives["root"] = "webpage";
	
	return 0;
}

int ConfigurationFile::_validatePort(const std::string& port) {
	std::istringstream iss(port);
    size_t portNum;
    
    if (!(iss >> portNum) || !iss.eof()) {
        throw ErrorInvalidPort("Port must be a valid number");
    }
    
    if (portNum > 65535) {
        throw ErrorInvalidPort("Port number must be between 0 and 65535");
    }
    
    if (std::find(_ports.begin(), _ports.end(), portNum) != _ports.end()) {
        throw ErrorInvalidPort("Port " + port + " is already in use");
    }
    
    _ports.push_back(portNum);
    return 0;
}

const ServerConfigs& ConfigurationFile::getServers(void) const {
	return _servers;
}

const std::vector<size_t>& ConfigurationFile::getPorts(void) const {
	return _ports;
}
