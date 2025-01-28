/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigFile.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aneitenb <aneitenb@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/28 09:56:54 by aneitenb          #+#    #+#             */
/*   Updated: 2025/01/28 14:19:54 by aneitenb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ConfigFile.hpp"

ConfigurationFile::ConfigurationFile(void) {}

ConfigurationFile::~ConfigurationFile(void) {
	if (_configFile.is_open()) {
		_configFile.close();
	}
}

const char* ConfigurationFile::ErrorOpeningConfFile::what(void) const throw() {
	return "Could not open configuration file";
}

const char* ConfigurationFile::ErrorInvalidConfig::what() const throw () {
	return "Invalid configuration format";
}
	
const char* ConfigurationFile::ErrorInvalidPort::what() const throw() {
	return "Invalid port number";
}

void ConfigurationFile::initializeConfFile(const std::string& filename) {
	_configFile.open(filename.c_str());
	if (_configFile.fail())
		throw ErrorOpeningConfFile();
	if (_readFile() == -1)
		throw ErrorInvalidConfig();
	if (_parseConfigFile() == -1)
		throw ErrorInvalidConfig();
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
		return -1;
		
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
	directives["root"] = "./www";
	
	return 0;
}

int ConfigurationFile::_validatePort(const std::string& port) {
	std::istringstream iss(port);
	size_t portNum;
	
	iss >> portNum;
	if (iss.fail() || portNum > 65535)
		return -1;
		
	_ports.push_back(portNum);
	return 0;
}

const ServerConfigs& ConfigurationFile::getServers(void) const {
	return _servers;
}

const std::vector<size_t>& ConfigurationFile::getPorts(void) const {
	return _ports;
}
