/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigFile.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aneitenb <aneitenb@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/28 09:56:54 by aneitenb          #+#    #+#             */
/*   Updated: 2025/02/12 16:21:00 by aneitenb         ###   ########.fr       */
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
	if (filename.empty()) {
		_setupDefaultServer();
		return;
	}
	
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
	size_t commentPos;
	size_t pathStart;
	size_t pathEnd;
	size_t delimiter;
	std::string key;
	std::string value;
	
	std::istringstream iss(_configContent);
	inServerBlock = false;
	inLocationBlock = false;
	bracketCount = 0;

	// Read file line by line
	while (std::getline(iss, line)) {
		line = _trimWhitespace(line);
		// Skip empty lines and comments
		if (line.empty() || line[0] == '#')
			continue;

		// Remove inline comments
		commentPos = line.find('#');
		if (commentPos != std::string::npos) {
			line = _trimWhitespace(line.substr(0, commentPos));
		}

		// Check for server block start
		if (line == "server {" || line == "server{") {
			if (inServerBlock)
				throw ErrorInvalidConfig("Nested server blocks not allowed");
			inServerBlock = true;
			bracketCount++;
			currentServer.clear();
			continue;
		}

		 // Location block start
		if (inServerBlock && line.substr(0, 9) == "location ") {
			inLocationBlock = true;
			
			// Extract location path and validate format
			pathStart = 9;  // After "location "
			pathEnd = line.find('{');
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

			delimiter = line.find(' ');
			if (delimiter == std::string::npos)
				throw ErrorInvalidConfig("Invalid directive format: " + line);

			key = line.substr(0, delimiter);
			value = _trimWhitespace(line.substr(delimiter + 1));
			
			if (!_isValidDirective(key) && key.substr(0, 9) != "location_")
				throw ErrorInvalidConfig("Unknown directive: " + key);

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

	// Check if we ended with an unclosed server block
	if (inServerBlock)
		throw ErrorInvalidConfig("Unclosed server block");

	// Check if we found any servers
	if (_servers.empty())
		throw ErrorInvalidConfig("No valid server blocks found");

	return 0;
}

const std::set<std::string> ConfigurationFile::_validDirectives = {
	// Server level directives
	"listen",
	"host",
	"root",
	"server_name",
	"client_max_body_size",
	"error_page",
	"index",
	
	// Location level directives
	"return",
	"autoindex",
	"cgi_pass",
	"cgi_param",
	"allowed_methods",
	"upload_store",
	"alias"
};

bool ConfigurationFile::_isValidDirective(const std::string& directive) const {
	return _validDirectives.find(directive) != _validDirectives.end();
}

bool ConfigurationFile::_checkPermissions(const std::string& path, bool writeAccess) const {
	struct stat buffer;	//system structure defined in sys/stat.h contains info about file
	
	// Stat gets file status, takes: file path (in c style string), where to store. Returns 0 on success
	if (stat(path.c_str(), &buffer) != 0)
		return false;
		
	// Check read access: path, mode to look for
	if (access(path.c_str(), R_OK) != 0)
		return false;
		
	// Check write access if needed
	if (writeAccess && access(path.c_str(), W_OK) != 0)
		return false;
		
	// Check if it's a directory (S_ISDIR macro that checks if file type bits in st_mode of struct stat indicate directory)
	if (S_ISDIR(buffer.st_mode)) {
		// Directories need execute permission to list contents
		if (access(path.c_str(), X_OK) != 0)
			return false;
	}
	
	return true;
}

std::string ConfigurationFile::_trimWhitespace(const std::string& str) const {
	const std::string WHITESPACE = " \n\r\t\f\v";
	size_t start;
	size_t end;

	// Find first non-whitespace character
	start = str.find_first_not_of(WHITESPACE);
	
	// If string is all whitespace, return empty string
	if (start == std::string::npos) 
		return "";
	
	// Find last non-whitespace character
	end = str.find_last_not_of(WHITESPACE);
	
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
	// Validate each directive
	for (ServerBlocks::const_iterator it = directives.begin(); it != directives.end(); ++it) {
		std::string directive = it->first;
		std::string value = it->second;
		
		// Skip location block directives (they start with "location_")
		if (directive.substr(0, 9) == "location_")
			continue;
			
		// Check if directive is valid
		if (!_isValidDirective(directive))
			throw ErrorInvalidConfig("Unknown directive: " + directive);
			
		// Validate listen directive (port)
		if (directive == "listen" && !_isValidPort(value))
			throw ErrorInvalidPort("Invalid port number");
			
		// Validate host directive (IP)
		if (directive == "host" && !_isValidIP(value))
			throw ErrorInvalidIP("Invalid IP address");
			
		// Check root path and permissions
		if (directive == "root") {
			if (!_isValidPath(value))
				throw ErrorInvalidConfig("Invalid root path");
			if (!_checkPermissions(value, true))
				throw ErrorInvalidConfig("Insufficient permissions for root path: " + value);
		}
		
		// Validate CGI paths if present
		// if (directive == "cgi_pass" && !_checkPermissions(value))
		// 	throw ErrorInvalidConfig("Invalid CGI executable or insufficient permissions: " + value);
		// if (directive == "cgi_param" && !_checkPermissions(value))
		// 	throw ErrorInvalidConfig("Invalid CGI executable or insufficient permissions: " + value);
	}
	return true;
}

void ConfigurationFile::_setupDefaultServer(void) {
	ServerBlocks	defaultServer;
	
	defaultServer["listen"] = "8080";
	defaultServer["host"] = "127.0.0.1";
	defaultServer["server_name"] = "localhost";
	defaultServer["root"] = "webpage";
	defaultServer["autoindex"] = "off";
	defaultServer["client_max_body_size"] = "1048576";	// 1 megabyte
	
	if (_validateServerBlock(defaultServer)) {
		_addPort(defaultServer["listen"]);
		_servers.push_back(defaultServer);
	}
}

bool ConfigurationFile::_isValidIP(const std::string& ip) const {
	std::istringstream iss(ip);
	std::string segment;
	int count;
	int value;

	count = 0;
	while (std::getline(iss, segment, '.'))
	{
		if (++count > 4)
			return false;
		if (segment.empty() || segment.length() > 3)
			return false;
		for (std::string::const_iterator it = segment.begin(); it != segment.end(); ++it)
		{
			if (!std::isdigit(*it)) 
			return false;
		}

		value = std::atoi(segment.c_str());
		if (value < 0 || value > 255) 
			return false;
	}
	return count == 4;	//boolean expression (equality operator): if count is equal to 4, then it is true
}

//just checks if port is valid
bool ConfigurationFile::_isValidPort(const std::string& port) const {
	std::istringstream iss(port);
	size_t portNum;
	
	// Check if it's a valid number
	if (!(iss >> portNum) || !iss.eof())	// >> extracts data from the stream and converts it to the target type, eof checks no extra data after number
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
	
	if (std::find(_ports.begin(), _ports.end(), portNum) != _ports.end()) {	//returns _ports.end if port is NOT found(good)
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

bool ConfigurationFile::_isAutoindexValid(const std::string& value) const{
	if (value == "on")
		return true;
	if (value == "off")
		return true;
	return false;
}

std::string ConfigurationFile::_getValue(const ServerBlocks& directives, const std::string& key) const {
	ServerBlocks::const_iterator it;
	
	it = directives.find(key);
	if (it != directives.end())	//If find() can't find the key, it returns this end() position
		return it->second;	//it->first gets the key, it->second gets the value
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
