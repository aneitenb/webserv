/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigFile.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aneitenb <aneitenb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/28 09:56:54 by aneitenb          #+#    #+#             */
/*   Updated: 2025/04/07 16:48:41 by aneitenb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config/ConfigFile.hpp"
#include "config/ConfigErrors.hpp"

const std::set<std::string> ConfigurationFile::_serverOnlyDirectives = {
	"listen",
	"host",
	"server_name",
	"client_max_body_size",
	"error_page"
};

const std::set<std::string> ConfigurationFile::_locationOnlyDirectives = {
	"return",
	"autoindex",
	"cgi_pass",
	"cgi_param",
	"upload_store",
	"alias"
};

const std::set<std::string> ConfigurationFile::_commonDirectives = {
	"root",
	"index",
	"allowed_methods"
};

ConfigurationFile::ConfigurationFile(void) {
}

ConfigurationFile::~ConfigurationFile(void) {
	if (_configFile.is_open()) {
		_configFile.close();
	}
}

void ConfigurationFile::initialize(const std::string& filename) {
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
	ServerBlock currentServer;
	LocationBlock currentLocationBlock;
	bool inServerBlock = false;
	bool inLocationBlock = false;
	std::string currentLocation;
	int bracketCount = 0;
	std::map<std::string, bool> locationBlocksStarted;

	std::istringstream iss(_configContent);

	while (std::getline(iss, line)) {
		line = _trimWhitespace(line);
		//skip empty lines and comments
		if (line.empty() || line[0] == '#')
			continue;

		// remove inline comments
		size_t commentPos = line.find('#');
		if (commentPos != std::string::npos) {
			line = _trimWhitespace(line.substr(0, commentPos));
		}

		// check for server block start
		if (line == "server {" || line == "server{") {
			if (inServerBlock)
				throw ErrorInvalidConfig("Nested server blocks not allowed");
			inServerBlock = true;
			bracketCount++;
			currentServer.clear();
			continue;
		}

		//location block start
		if (inServerBlock && line.substr(0, 9) == "location ") {
			inLocationBlock = true;
			
			// get location path and validate format
			size_t pathStart = 9;  //after "location "
			size_t pathEnd = line.find('{');
			if (pathEnd == std::string::npos)
				throw ErrorInvalidConfig("Invalid location block format: " + line);
				
			currentLocation = _trimWhitespace(line.substr(pathStart, pathEnd - pathStart));
			if (currentLocation.empty())
				throw ErrorInvalidConfig("Empty location path: " + line);
			if (currentLocation.length() > MAX_ROOT_PATH_LENGTH)
				throw ErrorInvalidConfig("Location path too long (max " + std::to_string(MAX_ROOT_PATH_LENGTH) + " characters)");
			
			//check if this location has already been defined in this server block
			if (locationBlocksStarted.find(currentLocation) != locationBlocksStarted.end()) {
				throw ErrorInvalidConfig("Duplicater location block for path: " + currentLocation);
			}
			
			//mark location as started
			locationBlocksStarted[currentLocation] = true;
			currentLocationBlock = LocationBlock();
			bracketCount++;
			continue;
		}

		// Handle closing brackets
		if (line == "}") {
			if (!inServerBlock)
				throw ErrorInvalidConfig("Unexpected closing bracket");
			
			bracketCount--;
			
			if (inLocationBlock && bracketCount == 1) {
				//end of location block, add complete block
				currentServer.addLocationBlock(currentLocation, currentLocationBlock);
				inLocationBlock = false;
				currentLocation.clear();
			}
			else if (bracketCount == 0) {
				//end of server block
				if (_validateServerBlock(currentServer)) {
					_servers.push_back(currentServer);
				inServerBlock = false;
					locationBlocksStarted.clear();
				}
			}
			continue;
		}

		// Parse directives
		if (inServerBlock && !line.empty()) {
			//skip opening braces
			if (line == "{")
				continue;

			size_t delimiter = line.find(' ');
			if (delimiter == std::string::npos)
				throw ErrorInvalidConfig("Invalid directive format: " + line);

			std::string key = line.substr(0, delimiter);
			std::string value = _trimWhitespace(line.substr(delimiter + 1));
			
			// only check for semicolons on regular directives, not on block starts
			if (!value.empty() && value[value.length() - 1] == ';') {
				value = _trimWhitespace(value.substr(0, value.length() - 1));

				if (inLocationBlock) {
					_parseLocationDirective(currentServer, currentLocationBlock, key, value);
				} else {
					_parseServerDirective(currentServer, key, value);
				}
			}
			else if (!inLocationBlock && !line.empty() && line.find("{") == std::string::npos) {
				//throw semicolon error for non-block directives
				throw ErrorInvalidConfig("Missing semicolon in directive: " + line);
			}
		}
	}

	// check if we ended with an unclosed server block
	if (inServerBlock)
		throw ErrorInvalidConfig("Unclosed server block");

	// check if we found any servers
	if (_servers.empty())
		throw ErrorInvalidConfig("No valid server blocks found");

	_checkForDuplicateServers();
	return 0;
}

bool ConfigurationFile::_checkForDuplicateServers() const {
	for (size_t i = 0; i < _servers.size(); ++i) {
		const ServerBlock& serverA = _servers[i];
		
		for (size_t j = i + 1; j < _servers.size(); ++j) {
			const ServerBlock& serverB = _servers[j];
			
			if (serverA.getServerName() != serverB.getServerName())
				continue;
			if (serverA.getHost() != serverB.getHost())
				continue;
				
			const std::vector<std::string>& portsA = serverA.getListen();
			const std::vector<std::string>& portsB = serverB.getListen();
			
			for (size_t portIndexA = 0; portIndexA < portsA.size(); ++portIndexA) {
				for (size_t portIndexB = 0; portIndexB < portsB.size(); ++portIndexB) {
					if (portsA[portIndexA] == portsB[portIndexB]) {
						throw ErrorInvalidConfig("Duplicate server configuration: same host (" + 
							serverA.getHost() + "), port (" + portsA[portIndexA] + 
							"), and server_name (" + serverA.getServerName() + ")");
					}
				}
			}
		}
	}	
	return true;
}

void ConfigurationFile::_parseServerDirective(ServerBlock& server, const std::string& key, const std::string& value) {
	if (!_isValidServerDirective(key))
		throw ErrorInvalidConfig("Unknown directive: " + key);
		
	if (key == "listen") {
		if (!_isValidPort(value))
			throw ErrorInvalidPort("Invalid port number: " + value);
		if (server.hasPort(value))
			throw ErrorInvalidConfig("Duplicate port in server block: " + value);
		server.addListen(value);
	}
	else if (key == "host") {
		if (server.hasHost())
			throw ErrorInvalidConfig("Duplicate 'host' directive in server block");
		if (!_isValidIP(value))
			throw ErrorInvalidIP("Invalid IP address: " + value);
		server.setHost(value);
	}
	else if (key == "root") {
		if (server.hasRoot())
			throw ErrorInvalidConfig("Duplicate 'root' directive in server block");
		if (!_isValidPath(value))
			throw ErrorInvalidConfig("Invalid root path: " + value);
		if (!_checkPermissions(value, true))
			throw ErrorInvalidConfig("Insufficient permissions for root path: " + value);
		if (value.length() > MAX_ROOT_PATH_LENGTH)
			throw ErrorInvalidConfig("Root path too long (max " + std::to_string(MAX_ROOT_PATH_LENGTH) + " characters)");
		server.setRoot(value);
	}
	else if (key == "server_name") {
		if (server.hasServerName())
			throw ErrorInvalidConfig("Duplicate 'server_name' directive in server block");
		if (value.length() > MAX_SERVER_NAME_LENGTH)
			throw ErrorInvalidConfig("Server name too long (max " + std::to_string(MAX_SERVER_NAME_LENGTH) + " characters)");
		if (!_isValidHostname(value))
			throw ErrorInvalidConfig("Invalid server name format: " + value);
		server.setServerName(value);
	}
	else if (key == "client_max_body_size") {
		if (server.hasClientMaxBodySize())
			throw ErrorInvalidConfig("Duplicate 'client_max_body_size' directive in server block");
		size_t size;
		try {
			size = std::stoull(value);
		} catch (const std::exception& e) {
			throw ErrorInvalidConfig("Invalid client_max_body_size value: " + value);
		}
		
		if (size > MAX_BODY_SIZE)
			throw ErrorInvalidConfig("client_max_body_size exceeds maximum allowed value");
			
		server.setClientMaxBodySize(size);
	}
	else if (key == "error_page") {
		// Format: error_page 404 /404.html;
		std::istringstream ess(value);
		int status;
		std::string path;
		
		if (!(ess >> status >> path) || status < 100 || status > 599)
			throw ErrorInvalidConfig("Invalid error_page format: " + value);
			
		// check if error page exists
		std::string fullPath = server.getRoot() + path;
		if (!_isValidPath(fullPath))
			throw ErrorInvalidConfig("Invalid error page path: " + fullPath);
		if (!_checkPermissions(fullPath, false))
			throw ErrorInvalidConfig("Insufficient permissions for error page: " + fullPath);
			
		server.addErrorPage(status, path);
	}
	else if (key == "index") {
		if (server.hasIndex())
			throw ErrorInvalidConfig("Duplicate 'index' directive in server block");
		if (value.length() > MAX_ROOT_PATH_LENGTH)
			throw ErrorInvalidConfig("Index path too long (max " + std::to_string(MAX_ROOT_PATH_LENGTH) + " characters)");
		//check if index file exists in server's root
		std::string fullPath = server.getRoot() + "/" + value;
		if (!_isValidPath(fullPath))
			throw ErrorInvalidConfig("Specified index file does not exist: " + fullPath);
		if (!_checkPermissions(fullPath, false))
			throw ErrorInvalidConfig("Insufficient permissions for index file: " + fullPath);
		server.setIndex(value);
	}
	else if (key == "allowed_methods") {
		if (server.hasAllowedMethods())
			throw ErrorInvalidConfig("Duplicate 'allowed_methods' directive in server block");
		uint8_t methods = _parseHttpMethods(value);
		if (methods == 0)
			throw ErrorInvalidConfig("No valid HTTP methods specified: " + value);
		server.setAllowedMethods(methods);
	}
}

void ConfigurationFile::_parseLocationDirective(ServerBlock& server, LocationBlock& locBlock, 
												const std::string& key, const std::string& value) {
	if (!_isValidLocationDirective(key))
		throw ErrorInvalidConfig("Unknown directive: " + key);
	
	if (key == "return") {
		if (locBlock.hasRedirect())
			throw ErrorInvalidConfig("Duplicate 'return' directive in location block");
		// Format: return 301 http://example.com;
		std::istringstream ress(value);
		int status;
		std::string url;
		
		if (!(ress >> status >> url) || status < 300 || status > 399)
			throw ErrorInvalidConfig("Invalid return format: " + value);
		if (url.length() > MAX_PATH_LENGTH) {
			throw ErrorInvalidConfig("Redirect URL too long (max " + 
				std::to_string(MAX_PATH_LENGTH) + " characters)");
		}
			
		locBlock.setRedirect(std::make_pair(status, url));
	}
	else if (key == "autoindex") {
		if (locBlock.hasAutoindex())
			throw ErrorInvalidConfig("Duplicate 'autoindex' directive in location block");
		if (value != "on" && value != "off")
			throw ErrorInvalidConfig("autoindex must be 'on' or 'off': " + value);
			
		locBlock.setAutoindex(value == "on");
	}
	else if (key == "cgi_pass") {
		if (locBlock.hasCgiPass())
			throw ErrorInvalidConfig("Duplicate 'cgi_pass' directive in location block");
		// Check if CGI executable exists and is executable
		if (!_isValidPath(value))
			throw ErrorInvalidConfig("Invalid CGI executable path: " + value);
		struct stat buffer;
		if (stat(value.c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode))
			throw ErrorInvalidConfig("CGI path cannot be a directory: " + value);
		if (!_checkPermissions(value, false))
			throw ErrorInvalidConfig("Insufficient permissions for CGI executable: " + value);
		if (value.length() > MAX_ROOT_PATH_LENGTH)
			throw ErrorInvalidConfig("CGI path too long (max " + std::to_string(MAX_ROOT_PATH_LENGTH) + " characters)");
			
		locBlock.setCgiPass(value);
	}
	else if (key == "cgi_param") {
		if (locBlock.hasCgiParam(value))
			throw ErrorInvalidConfig("Duplicate 'cgi_param' directive in location block");
		// Format: cgi_param SCRIPT_FILENAME $document_root$fastcgi_script_name;
		size_t paramDelim = value.find(' ');
		if (paramDelim == std::string::npos)
			throw ErrorInvalidConfig("Invalid cgi_param format: " + value);
			
		std::string paramName = value.substr(0, paramDelim);
		std::string paramValue = _trimWhitespace(value.substr(paramDelim + 1));
		
		locBlock.setCgiParam(paramName, paramValue);
	}
	else if (key == "allowed_methods") {
		if (locBlock.hasAllowedMethods())
			throw ErrorInvalidConfig("Duplicate 'allowed_methods' directive in location block");
		uint8_t methods = _parseHttpMethods(value);
		if (methods == 0)
			throw ErrorInvalidConfig("No valid HTTP methods specified: " + value);
			
		locBlock.setAllowedMethods(methods);
	}
	// else if (key == "upload_store") {
	// 	if (locBlock.hasUploadStore())
	// 		throw ErrorInvalidConfig("Duplicate 'upload_store' directive in location block");
		
	// 	// Check if directory exists or create it
	// 	if (!_isValidPath(value)) {
	// 		// Directory doesn't exist, check if we can create it
	// 		if (mkdir(value.c_str(), 0755) != 0) {
	// 			throw ErrorInvalidConfig("Cannot create upload directory: " + value);
	// 		}
	// 	}
		
	// 	// Check if path is a directory
	// 	struct stat st;
	// 	if (stat(value.c_str(), &st) == 0 && !S_ISDIR(st.st_mode)) {
	// 		throw ErrorInvalidConfig("Upload store path is not a directory: " + value);
	// 	}
		
	// 	if (value.length() > MAX_ROOT_PATH_LENGTH)
	// 		throw ErrorInvalidConfig("Upload store path too long (max " + std::to_string(MAX_ROOT_PATH_LENGTH) + " characters)");
	// 	if (!_checkPermissions(value, true))
	// 		throw ErrorInvalidConfig("Insufficient permissions for upload directory: " + value);
	// 	locBlock.setUploadStore(value);
	// }
	else if (key == "upload_store") {
		if (locBlock.hasUploadStore())
			throw ErrorInvalidConfig("Duplicate 'upload_store' directive in location block");
		
		std::string uploadPath = value;
		std::string fullPath;
		
		// If it's an absolute path
		if (value[0] == '/') {
			fullPath = value;
		} else {
			// It's a relative path, make it relative to server root
			fullPath = server.getRoot() + "/" + value;
		}
		
		std::cerr << "Trying to use upload directory: " << fullPath << std::endl;
		
		// Check if directory exists
		struct stat st;
		if (stat(fullPath.c_str(), &st) == 0) {
			// Directory exists, check if it's actually a directory
			if (!S_ISDIR(st.st_mode)) {
				throw ErrorInvalidConfig("Upload store path exists but is not a directory: " + fullPath);
			}
			
			// Try to chmod the directory to ensure we have write permissions
			std::cerr << "Directory exists, attempting to set permissions to 0755" << std::endl;
			if (chmod(fullPath.c_str(), 0755) != 0) {
				std::cerr << "Warning: Could not change permissions on " << fullPath 
						 << " (Error: " << std::endl;
			}
		} else {
			// Directory doesn't exist, check parent directory
			std::string parentDir = fullPath.substr(0, fullPath.find_last_of('/'));
			if (parentDir.empty() && fullPath[0] == '/') parentDir = "/";
			
			std::cerr << "Directory doesn't exist. Checking parent directory: " << parentDir << std::endl;
			
			// Check if parent directory exists
			if (stat(parentDir.c_str(), &st) == 0) {
				// Parent exists, try to chmod it to ensure we have write permissions
				std::cerr << "Parent directory exists, attempting to set permissions to 0755" << std::endl;
				if (chmod(parentDir.c_str(), 0755) != 0) {
					std::cerr << "Warning: Could not change permissions on " << parentDir 
							 << " (Error: " << std::endl;
				}
				
				// Try to create the directory
				if (mkdir(fullPath.c_str(), 0755) != 0) {
					throw ErrorInvalidConfig("Cannot create upload directory: " + fullPath + 
										  " (Error: ");
				}
			} else {
				// Parent doesn't exist
				throw ErrorInvalidConfig("Parent directory doesn't exist: " + parentDir);
			}
		}
		
		// Final check to ensure we have the required permissions
		if (access(fullPath.c_str(), R_OK | W_OK | X_OK) != 0) {
			std::cerr << "Warning: Directory exists but we don't have full RWX permissions: " 
					 << std::endl;
			
			// We'll continue anyway since we've made a best effort to set permissions
		}
		
		locBlock.setUploadStore(uploadPath); // Store the original path as specified in config
	}
	else if (key == "alias") {
		if (locBlock.hasAlias())
			throw ErrorInvalidConfig("Duplicate 'alias' directive in location block");
		// Check if alias path exists
		if (!_isValidPath(value))
			throw ErrorInvalidConfig("Invalid alias path: " + value);
		if (!_checkPermissions(value, false))
			throw ErrorInvalidConfig("Insufficient permissions for alias path: " + value);
			
		if (value.length() > MAX_ROOT_PATH_LENGTH)
			throw ErrorInvalidConfig("Alias path too long (max " + std::to_string(MAX_ROOT_PATH_LENGTH) + " characters)");
			
		locBlock.setAlias(value);
	}
	else if (key == "root") {
		if (locBlock.hasRoot())
			throw ErrorInvalidConfig("Duplicate 'root' directive in location block");
		if (!_isValidPath(value))
			throw ErrorInvalidConfig("Invalid root path: " + value);
		if (!_checkPermissions(value, true))
			throw ErrorInvalidConfig("Insufficient permissions for root path: " + value);
		if (value.length() > MAX_ROOT_PATH_LENGTH)
			throw ErrorInvalidConfig("Root path too long (max " + std::to_string(MAX_ROOT_PATH_LENGTH) + " characters)");
		locBlock.setRoot(value);
	}
	else if (key == "index") {
		if (locBlock.hasIndex())
			throw ErrorInvalidConfig("Duplicate 'index' directive in location block");
		if (value.length() > MAX_ROOT_PATH_LENGTH)
			throw ErrorInvalidConfig("Index path too long (max " + std::to_string(MAX_ROOT_PATH_LENGTH) + " characters)");
		//check if index file exists in server's root
		std::string fullPath = server.getRoot() + "/" + value;
		if (!_isValidPath(fullPath))
			throw ErrorInvalidConfig("Specified index file does not exist: " + fullPath);
		if (!_checkPermissions(fullPath, false))
			throw ErrorInvalidConfig("Insufficient permissions for index file: " + fullPath);
		locBlock.setIndex(value);
	}
}

bool ConfigurationFile::_validateServerBlock(const ServerBlock& server) const {
	if (server.getListen().size() == 0)
		throw ErrorInvalidConfig("Missing 'listen' directive");
	if (server.getHost().empty())
		throw ErrorInvalidConfig("Missing 'host' directive");
	if (server.getRoot().empty())
		throw ErrorInvalidConfig("Missing 'root' directive");

	
	if (!server.getIndex().empty()) {
		std::string fullIndexPath = server.getRoot() + "/" + server.getIndex();
		if (!_isValidPath(fullIndexPath))
			throw ErrorInvalidConfig("Default index file not found: " + fullIndexPath);
		if (!_checkPermissions(fullIndexPath, false))
			throw ErrorInvalidConfig("Insufficient read permissions for index file: " + fullIndexPath);
	}
		
	if (!server.hasCustomErrorPages()) {
		std::string defaultErrorDir = server.getRoot() + server.getDefaultErrorDir();
		if (!_isValidPath(defaultErrorDir)) {
			throw ErrorInvalidConfig("Default error directory not found: " + defaultErrorDir);
		}
		if (!_checkPermissions(defaultErrorDir, false)) {
			throw ErrorInvalidConfig("Insufficient permissions for default error directory: " + defaultErrorDir);
		}
		
		//checks that specific default error pages exist
		std::vector<int> requiredErrorCodes = {400, 403, 404, 500, 503};
		for (int code : requiredErrorCodes) {
			std::string errorPage = defaultErrorDir + "/" + std::to_string(code) + ".html";
			if (!_isValidPath(errorPage)) {
				throw ErrorInvalidConfig("Required default error page not found: " + errorPage);
			}
			if (!_checkPermissions(errorPage, false)) {
				throw ErrorInvalidConfig("Insufficient permissions for default error page: " + errorPage);
			}
		}
	}
	
	const std::map<std::string, LocationBlock>& locationBlocks = server.getLocationBlocks();
	for (std::map<std::string, LocationBlock>::const_iterator it = locationBlocks.begin(); 
		 it != locationBlocks.end(); ++it) {
		
		_validateLocationBlock(it->first, it->second);
	}
	
	return true;
}

bool ConfigurationFile::_validateLocationBlock(const std::string& path, const LocationBlock& block) const {
	if (block.hasCgiPass()) {
		if (!block.hasCgiParam("SCRIPT_FILENAME")) {
			throw ErrorInvalidConfig("CGI_PASS requires SCRIPT_FILENAME parameter for location: " + path);
		}
	}
	if (block.hasRoot() && block.hasAlias()) {
		throw ErrorInvalidConfig("Cannot use both root and alias directives in the same location: " + path);
	}
	return true;
}

std::string ConfigurationFile::_trimWhitespace(const std::string& str) const {
	const std::string WHITESPACE = " \n\r\t\f\v";
	size_t start = str.find_first_not_of(WHITESPACE);
	
	if (start == std::string::npos) 
		return "";
	size_t end = str.find_last_not_of(WHITESPACE);
	return str.substr(start, end - start + 1);
}

bool ConfigurationFile::_isValidIP(const std::string& ip) const {
	std::istringstream iss(ip);
	std::string segment;
	int count = 0;
	
	while (std::getline(iss, segment, '.')) {
		if (++count > 4)
			return false;
		if (segment.empty() || segment.length() > 3)
			return false;
			
		for (std::string::const_iterator it = segment.begin(); it != segment.end(); ++it) {
			if (!std::isdigit(*it)) 
				return false;
		}

		int value = std::atoi(segment.c_str());
		if (value < 0 || value > 255) 
			return false;
	}
	
	return count == 4;
}

bool ConfigurationFile::_isValidHostname(const std::string& hostname) const {
	//checks for valid characters and proper formatting using regex
	std::regex hostnamePattern("^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9])\\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\\-]*[A-Za-z0-9])$");
	return std::regex_match(hostname, hostnamePattern);
}

bool ConfigurationFile::_isValidPort(const std::string& port) const {
	std::istringstream iss(port);
	size_t portNum;
	
	if (!(iss >> portNum) || !iss.eof())
		return false;
	if (portNum < 1 || portNum > 65535)
		return false;
	if (portNum < 1024 && geteuid() != 0) {
		return false;  //need root privileges for ports < 1024
	}
	return true;
}

bool ConfigurationFile::_isValidPath(const std::string& path) const {
	if (path.empty())
		return false;
		
	struct stat buffer;
	return stat(path.c_str(), &buffer) == 0;
}

bool ConfigurationFile::_checkPermissions(const std::string& path, bool writeAccess) const {
	struct stat buffer;
	
	if (stat(path.c_str(), &buffer) != 0)
		return false;
		
	if (access(path.c_str(), R_OK) != 0)
		return false;
		
	if (writeAccess && access(path.c_str(), W_OK) != 0)
		return false;
		
	if (S_ISDIR(buffer.st_mode)) {
		if (access(path.c_str(), X_OK) != 0)
			return false;
	}
	
	return true;
}

bool ConfigurationFile::_isValidServerDirective(const std::string& directive) const {
	return _serverOnlyDirectives.find(directive) != _serverOnlyDirectives.end() ||
			_commonDirectives.find(directive) != _commonDirectives.end();
}

bool ConfigurationFile::_isValidLocationDirective(const std::string& directive) const {
	return _locationOnlyDirectives.find(directive) != _locationOnlyDirectives.end() ||
			_commonDirectives.find(directive) != _commonDirectives.end();
}

uint8_t ConfigurationFile::_parseHttpMethods(const std::string& methods) {
	uint8_t result = 0;
	std::istringstream iss(methods);
	std::string method;
	
	while (iss >> method) {
		if (method == "GET")
			result |= GET;
		else if (method == "POST")
			result |= POST;
		else if (method == "DELETE")
			result |= DELETE;
		else
			throw ErrorInvalidConfig("Unknown HTTP method: " + method);
	}
	
	return result;
}

ServerBlock ConfigurationFile::getServerBlock(size_t index) const {
	if (index >= _servers.size())
		throw std::out_of_range("Server index out of range");
	return _servers[index];
}

//returns first server matching IP and port
const ServerBlock& ConfigurationFile::getServerBlockByIPPort(
		const std::string& ipAddress, const std::string& port) const {
	
	for (size_t i = 0; i < _servers.size(); ++i) {
		const ServerBlock& server = _servers[i];
		const std::vector<std::string>& serverPorts = server.getListen();
		
		//If the port is not found anywhere in the vector, std::find returns serverPorts.end(), 
		//which is a special iterator position that points just past the end of the vector
		if (std::find(serverPorts.begin(), serverPorts.end(), port) != serverPorts.end()) {
			// IP matches or server is set to listen on all interfaces
			if (server.getHost() == ipAddress || server.getHost() == "0.0.0.0") {
				return server; //return the first matching server
			}
		}
	}
	throw ErrorNoMatchingServer("No server configured for IP:port " + ipAddress + ":" + port);
}

//returns all servers matching IP and port
std::vector<const ServerBlock*> ConfigurationFile::getAllServerBlocksByIPPort(
		const std::string& ipAddress, const std::string& port) const {
	
	std::vector<const ServerBlock*> matchingServers;
	
	for (size_t i = 0; i < _servers.size(); ++i) {
		const ServerBlock& server = _servers[i];
		const std::vector<std::string>& serverPorts = server.getListen();
		
		if (std::find(serverPorts.begin(), serverPorts.end(), port) != serverPorts.end()) {
			if (server.getHost() == ipAddress || server.getHost() == "0.0.0.0") {
				matchingServers.push_back(&_servers[i]);
			}
		}
	}
	if (matchingServers.empty())
		throw ErrorNoMatchingServer("No server configured for IP:port " + ipAddress + ":" + port);
	return matchingServers;
}

// returns server matching IP, port, and server name
const ServerBlock& ConfigurationFile::getServerBlockByHostPortName(
		const std::string& ipAddress, const std::string& port, 
		const std::string& serverName) const {
	
	// get all matching servers by IP and port
	std::vector<const ServerBlock*> matchingServers = 
		getAllServerBlocksByIPPort(ipAddress, port);
	if (matchingServers.empty()) {
		throw ErrorNoMatchingServer("No server configured for IP:port " + ipAddress + ":" + port);
	}
	
	// find one with matching server name
	for (size_t i = 0; i < matchingServers.size(); ++i) {
		if (matchingServers[i]->getServerName() == serverName) {
			return *matchingServers[i];
		}
	}
	
	// If no exact server name match, return the first server (default)
	return *matchingServers[0];
}

size_t ConfigurationFile::getServerCount() const {
	return _servers.size();
}
