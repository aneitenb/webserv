// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<ConfigFile.cpp>> -- <<Aida, Ilmari, Milica>>

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
			if (inLocationBlock) 
				throw ErrorInvalidConfig("Nested location blocks are not allowed");
			inLocationBlock = true;
			
			// get location path and validate format
			size_t pathStart = 9;  //after "location "
			size_t pathEnd = line.find('{');
			if (pathEnd == std::string::npos)
				throw ErrorInvalidConfig("Invalid location block format: " + line);
				
			currentLocation = _trimWhitespace(line.substr(pathStart, pathEnd - pathStart));
			if (!_isValidLocationPath(currentLocation))
    			throw ErrorInvalidConfig("Invalid location path format: " + currentLocation);
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

		std::istringstream iss(value);
		std::string rootPath;
		std::string extraToken;
		
		if (!(iss >> rootPath))
			throw ErrorInvalidConfig("Missing path in root directive");
		if (iss >> extraToken)
			throw ErrorInvalidConfig("Extra parameters not allowed in root directive: " + value);
			
		if (!_isValidPathFormat(rootPath))
			throw ErrorInvalidConfig("Invalid root path format: " + rootPath);
		if (rootPath.length() > MAX_ROOT_PATH_LENGTH)
			throw ErrorInvalidConfig("Root path too long (max " + std::to_string(MAX_ROOT_PATH_LENGTH) + " characters)");
		server.setRoot(rootPath);
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
		
		std::istringstream iss(value);
		size_t size;
		
		if (!(iss >> size))
			throw ErrorInvalidConfig("Invalid client_max_body_size value: " + value);

		std::string suffix;
		iss >> suffix;
		if (!suffix.empty()) {
			if (suffix.length() != 1)
				throw ErrorInvalidConfig("Invalid unit in client_max_body_size: " + value);
				
			char unit = std::tolower(suffix[0]);
			if (unit == 'k')
				size *= 1024;
			else if (unit == 'm')
				size *= 1024 * 1024;
			else if (unit == 'g')
				size *= 1024 * 1024 * 1024;
			else
				throw ErrorInvalidConfig("Invalid unit in client_max_body_size: " + value);
				
			//check if overflow occurred during multiplication
			if (size == 0)
				throw ErrorInvalidConfig("client_max_body_size value is too large: " + value);
		}

		std::string extraChars;
		if (iss >> extraChars)
			throw ErrorInvalidConfig("Invalid characters in client_max_body_size: " + value);

		if (size > MAX_BODY_SIZE)
			throw ErrorInvalidConfig("client_max_body_size exceeds maximum allowed value");
		
		server.setClientMaxBodySize(size);
	}
	else if (key == "error_page") {
		// Format: error_page 404 /404.html;
		std::istringstream ess(value);
		int status;
		std::string path;
		std::string extraToken;
		
		if (!(ess >> status >> path) || status < 100 || status > 599)
			throw ErrorInvalidConfig("Invalid error_page format: " + value);
		if (ess >> extraToken)
			throw ErrorInvalidConfig("Extra parameters not allowed in error_page directive: " + value);
		if (!_isValidPathFormat(path))
			throw ErrorInvalidConfig("Invalid error page path: " + path);
		server.addErrorPage(status, path);
	}
	else if (key == "index") {
		if (server.hasIndex())
			throw ErrorInvalidConfig("Duplicate 'index' directive in server block");
		if (value.length() > MAX_ROOT_PATH_LENGTH)
			throw ErrorInvalidConfig("Index path too long (max " + std::to_string(MAX_ROOT_PATH_LENGTH) + " characters)");
		if (!_isValidFilenameFormat(value))
			throw ErrorInvalidConfig("Invalid index file name: " + value);
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
		if (!_isValidPathFormat(value))
			throw ErrorInvalidConfig("Invalid CGI executable path: " + value);
		if (value.length() > MAX_ROOT_PATH_LENGTH)
			throw ErrorInvalidConfig("CGI path too long (max " + std::to_string(MAX_ROOT_PATH_LENGTH) + " characters)");
			
		locBlock.setCgiPass(value);
	}
	else if (key == "allowed_methods") {
		if (locBlock.hasAllowedMethods())
			throw ErrorInvalidConfig("Duplicate 'allowed_methods' directive in location block");
		uint8_t methods = _parseHttpMethods(value);
		if (methods == 0)
			throw ErrorInvalidConfig("No valid HTTP methods specified: " + value);
			
		locBlock.setAllowedMethods(methods);
	}
	else if (key == "upload_store") {
		if (locBlock.hasUploadStore())
			throw ErrorInvalidConfig("Duplicate 'upload_store' directive in location block");

		if (!_isValidPathFormat(value))
			throw ErrorInvalidConfig("Invalid upload store path format: " + value);
		
		if (value.length() > MAX_ROOT_PATH_LENGTH)
			throw ErrorInvalidConfig("Upload store path too long (max " + std::to_string(MAX_ROOT_PATH_LENGTH) + " characters)");
		
		locBlock.setUploadStore(value);
	}
	else if (key == "alias") {
		if (locBlock.hasAlias())
			throw ErrorInvalidConfig("Duplicate 'alias' directive in location block");
		if (!_isValidPathFormat(value))
			throw ErrorInvalidConfig("Invalid alias path: " + value);
		if (value.length() > MAX_ROOT_PATH_LENGTH)
			throw ErrorInvalidConfig("Alias path too long (max " + std::to_string(MAX_ROOT_PATH_LENGTH) + " characters)");
		locBlock.setAlias(value);
	}
	else if (key == "root") {
		if (locBlock.hasRoot())
			throw ErrorInvalidConfig("Duplicate 'root' directive in server block");

		std::istringstream iss(value);
		std::string rootPath;
		std::string extraToken;
		
		if (!(iss >> rootPath))
			throw ErrorInvalidConfig("Missing path in root directive");
		if (iss >> extraToken)
			throw ErrorInvalidConfig("Extra parameters not allowed in root directive: " + value);
			
		if (!_isValidPathFormat(rootPath))
			throw ErrorInvalidConfig("Invalid root path format: " + rootPath);
		if (rootPath.length() > MAX_ROOT_PATH_LENGTH)
			throw ErrorInvalidConfig("Root path too long (max " + std::to_string(MAX_ROOT_PATH_LENGTH) + " characters)");
		server.setRoot(rootPath);
	}
	else if (key == "index") {
		if (locBlock.hasIndex())
			throw ErrorInvalidConfig("Duplicate 'index' directive in location block");
		if (value.length() > MAX_ROOT_PATH_LENGTH)
			throw ErrorInvalidConfig("Index path too long (max " + std::to_string(MAX_ROOT_PATH_LENGTH) + " characters)");
		if (!_isValidPathFormat(value))
			throw ErrorInvalidConfig("Specified index file does not exist: " + value);
		locBlock.setIndex(value);
	}
}

bool ConfigurationFile::_validateServerBlock(ServerBlock& server) const {
	if (server.getListen().size() == 0)
		throw ErrorInvalidConfig("Missing 'listen' directive");
	if (server.getHost().empty() && server.getServerName().empty())
		throw ErrorInvalidConfig("Missing 'host' and 'server_name' directives");
	if (server.getHost().empty() && !server.getServerName().empty())
		server.setHost("0.0.0.0");
	if (server.getRoot().empty())
		throw ErrorInvalidConfig("Missing 'root' directive");
	
	if (server.getIndex().empty())
		server.setIndex("index.html");
	const std::map<std::string, LocationBlock>& locationBlocks = server.getLocationBlocks();
	for (std::map<std::string, LocationBlock>::const_iterator it = locationBlocks.begin(); 
		 it != locationBlocks.end(); ++it) {
		
		_validateLocationBlock(it->first, it->second);
	}
	
	return true;
}

bool ConfigurationFile::_validateLocationBlock(const std::string& path, const LocationBlock& block) const {
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
	//check that it contains only digits
	for (size_t i = 0; i < port.length(); i++) {
		if (!std::isdigit(port[i]))
			return false;
	}
	std::istringstream iss(port);
	size_t portNum;
	
	if (!(iss >> portNum) || !iss.eof())
		return false;
	if (portNum < 1024 || portNum > 65535) //need root privileges for ports < 1024
		return false;
	return true;
}

bool ConfigurationFile::_isValidLocationPath(const std::string& path) const {
    if (path.empty() || path[0] == '~' || path[0] == '=') {
        return false;
    }
	for (size_t i = 1; i < path.length(); ++i) {
		if (path [i] == '/' && path[i - 1] =='/')
			return false;
	}
    std::regex pathRegex("^[a-zA-Z0-9._\\-\\/]+$");
    return std::regex_match(path, pathRegex);
}

bool ConfigurationFile::_isValidPathFormat(const std::string& path) const {
	if (path.empty())
		return false;
	for (size_t i = 1; i < path.length(); ++i) {
        if (path[i] == '/' && path[i-1] == '/') {
            return false;
        }
    }
	if (path[0] == '/') {
		std::regex absolutePathRegex("^\\/[a-zA-Z0-9._\\-\\/]+$");
		return std::regex_match(path, absolutePathRegex);
	} else {
		std::regex relativePathRegex("^[a-zA-Z0-9._\\-\\/]+$");
		return std::regex_match(path, relativePathRegex);
	}
}

bool ConfigurationFile::_isValidFilenameFormat(const std::string& filename) const {
	if (filename.empty())
		return false;
		
	// Filename should not contain slashes or invalid characters
	std::regex filenameRegex("^[a-zA-Z0-9._\\-]+$");
	return std::regex_match(filename, filenameRegex);
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


std::vector<ServerBlock>& ConfigurationFile::getAllServerBlocks(){
	return _servers;
}
