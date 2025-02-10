/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigFile.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aneitenb <aneitenb@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/28 09:56:54 by aneitenb          #+#    #+#             */
/*   Updated: 2025/02/10 17:01:29 by aneitenb         ###   ########.fr       */
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
		throw ErrorInvalidConfig("Invalid configuration format");
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
	std::string line;
	ServerBlocks currentServer;
	bool inServerBlock;
	bool inLocationBlock;
	std::string currentLocation;
	int bracketCount;
	// size_t delimiter;
	std::string key;
	std::string value;
	// int result;
	
	std::istringstream iss(_configContent);
	inServerBlock = false;
	inLocationBlock = false;
	bracketCount = 0;
	// result = 0;

	// Read file line by line
	while (std::getline(iss, line)) {
		// Remove leading/trailing whitespace
		line = _trimWhitespace(line);
		
		// Skip empty lines and comments
		if (line.empty() || line[0] == '#')
			continue;

		// Remove inline comments
		size_t commentPos = line.find('#');
		if (commentPos != std::string::npos) {
			line = _trimWhitespace(line.substr(0, commentPos));
		}

		// Check for server block start
		if (line == "server {" || line == "server{}") {
			if (inServerBlock)
				throw ErrorInvalidConfig("Nested server blocks not allowed");
			inServerBlock = true;
			bracketCount++;
			currentServer.clear();
			_setupDefaultValues(currentServer);
			continue;
		}

		 // Location block start
        if (inServerBlock && line.substr(0, 9) == "location ") {
            inLocationBlock = true;
            
            // Extract location path and validate format
            size_t pathStart = 9;  // After "location "
            size_t pathEnd = line.find('{');
            if (pathEnd == std::string::npos)
                throw ErrorInvalidConfig("Invalid location block format: " + line);
                
            currentLocation = _trimWhitespace(line.substr(pathStart, pathEnd - pathStart));
            if (currentLocation.empty())
                throw ErrorInvalidConfig("Empty location path: " + line);

            bracketCount++;
            continue;
        }

        // Handle closing brackets
        if (line == "}") {
            if (!inServerBlock)
                throw ErrorInvalidConfig("Unexpected closing bracket");
            
            bracketCount--;
            
            if (inLocationBlock && bracketCount == 1) {
                // End of location block
                inLocationBlock = false;
                currentLocation.clear();
            }
            else if (bracketCount == 0) {
                // End of server block
                if (_validateServerBlock(currentServer)) {
                    _addPort(currentServer["listen"]);
                    _servers.push_back(currentServer);
                }
                inServerBlock = false;
            }
            continue;
        }

// Parse directives
        if (inServerBlock && !line.empty()) {
            // Skip opening braces
            if (line == "{")
                continue;

            size_t delimiter = line.find(' ');
            if (delimiter == std::string::npos)
                throw ErrorInvalidConfig("Invalid directive format: " + line);

            std::string key = line.substr(0, delimiter);
            std::string value = _trimWhitespace(line.substr(delimiter + 1));

            // Only check for semicolons on regular directives, not on block starts
            if (!value.empty() && value[value.length() - 1] == ';') {
                value = value.substr(0, value.length() - 1);
                value = _trimWhitespace(value);

                if (inLocationBlock) {
                    // Store location directives with location path prefix
                    currentServer["location_" + currentLocation + "_" + key] = value;
                } else {
                    currentServer[key] = value;
                }
            }
            else if (!inLocationBlock && !line.empty() && line.find("{") == std::string::npos) {
                // Only throw semicolon error for non-block directives
                throw ErrorInvalidConfig("Missing semicolon in directive: " + line);
            }
        }
    }

	// 	// Check for server block end
	// 	if (line == "}") {
	// 		if (!inServerBlock)
	// 			throw ErrorInvalidConfig("Unexpected closing bracket");
		
	// 		// Validate server configuration before adding
	// 		if (_validateServerBlock(currentServer)) {
	// 			_addPort(currentServer["listen"]);  // Add port if validation passed
	// 			_servers.push_back(currentServer);
	// 		}
		
	// 		inServerBlock = false;
	// 		continue;
	// 	}

	// 	// Parse directives inside server block
	// 	if (inServerBlock) {
	// 		delimiter = line.find(' ');
	// 		if (delimiter == std::string::npos)
	// 			throw ErrorInvalidConfig("Invalid directive format: " + line);

	// 		 key = line.substr(0, delimiter);
	// 		 value = _trimWhitespace(line.substr(delimiter + 1));

	// 		// Remove semicolon at the end if present
	// 		if (!value.empty() && value[value.length() - 1] == ';') {
	// 			value = value.substr(0, value.length() - 1);
	// 			value = _trimWhitespace(value);  // Trim again after removing semicolon
	// 			currentServer[key] = value;
	// 		}
	// 		else if (!line.empty()){	 // Only throw error if line isn't empty after comment removal
	// 			throw ErrorInvalidConfig("Missing semicolon in directive: " + line);
	// 		}
	// 	}
	// }

	// Check if we ended with an unclosed server block
	if (inServerBlock)
		throw ErrorInvalidConfig("Unclosed server block");

	// Check if we found any servers
	if (_servers.empty())
		throw ErrorInvalidConfig("No valid server blocks found");

	return 0;
}

std::string ConfigurationFile::_trimWhitespace(const std::string& str) const {
	const std::string WHITESPACE = " \n\r\t\f\v";
	
	// Find first non-whitespace character
	size_t start = str.find_first_not_of(WHITESPACE);
	
	// If string is all whitespace, return empty string
	if (start == std::string::npos) 
		return "";
	
	// Find last non-whitespace character
	size_t end = str.find_last_not_of(WHITESPACE);
	
	// Return trimmed substring
	return str.substr(start, end - start + 1);
}

bool ConfigurationFile::_validateServerBlock(const ServerBlocks& directives) const {
	// Check required directives
	const std::string requiredDirectives[] = {"listen", "host", "root"};
	for (size_t i = 0; i < sizeof(requiredDirectives) / sizeof(requiredDirectives[0]); ++i) {
		if (directives.find(requiredDirectives[i]) == directives.end()) {
			throw ErrorInvalidConfig("Missing required directive: " + requiredDirectives[i]);
		}
	}
	
	// Validate listen directive (port)
	if (!_isValidPort(directives.at("listen")))
		throw ErrorInvalidPort("Invalid port number");
	
	// Validate host directive (IP)
	if (!_isValidIP(directives.at("host")))
		throw ErrorInvalidIP("Invalid IP address");
		
	// Validate root directive (path)
	if (!_isValidPath(directives.at("root")))
		throw ErrorInvalidConfig("Invalid root path");
	
	return true;
}

int ConfigurationFile::_setupDefaultValues(ServerBlocks& directives) {
	// Set default values for a basic server configuration
	directives["listen"] = "8080";
	directives["host"] = "127.0.0.1";
	directives["server_name"] = "localhost";
	directives["root"] = "webpage";
	directives["directory_listing"] = "off";
	directives["client_max_body_size"] = "1048576";  // 1MB default
	return 0;
}

bool ConfigurationFile::_isValidIP(const std::string& ip) const {
	std::istringstream iss(ip);
	std::string segment;
	int count;

	count = 0;
	while (std::getline(iss, segment, '.'))
	{
		if (++count >4)
			return false;
		if (segment.empty() || segment.length() > 3)
			return false;
		for (std::string::const_iterator it = segment.begin(); it != segment.end(); ++it)
		{
			if (!std::isdigit(*it)) 
			return false;
		}

		int value = std::atoi(segment.c_str());
		if (value < 0 || value > 255) 
			return false;
	}
	return count == 4;
}

//just checks if port is valid
bool ConfigurationFile::_isValidPort(const std::string& port) const {
	std::istringstream iss(port);
	size_t portNum;
	
	// Check if it's a valid number
	if (!(iss >> portNum) || !iss.eof())
		return false;
	
	// Check if it's in valid range
	if (portNum > 65535)
		return false;
		
	return true;
}

//adds the port
int ConfigurationFile::_addPort(const std::string& port) {
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

bool ConfigurationFile::_isValidPath(const std::string& path) const{
	if (path.empty())
		return false;
	return true;
}

bool ConfigurationFile::_isDirectoryListingValid(const std::string& value) const{
	if (value == "on")
		return true;
	if (value == "off")
		return true;
	return false;
}

std::string ConfigurationFile::_getValue(const ServerBlocks& directives, const std::string& key) const {
	ServerBlocks::const_iterator it = directives.find(key);
	if (it != directives.end())
		return it->second;	//return its value
	else 
		return "";	//return empty string if key word isn't found
}

void ConfigurationFile::_setValue(ServerBlocks& directives, const std::string& key, const std::string& value) {
	directives[key] = value;
}

bool ConfigurationFile::_hasValue(const ServerBlocks& directives, const std::string& key) const{
	ServerBlocks::const_iterator it = directives.find(key);
	
	if (it != directives.end())
		return true;	//key was found
	else
		return false;
}

const std::vector<ServerBlocks>& ConfigurationFile::getServers(void) const {
	return _servers;
}

const std::vector<size_t>& ConfigurationFile::getPorts(void) const {
	return _ports;
}
