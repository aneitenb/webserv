// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<ServerBlock.cpp>> -- <<Aida, Ilmari, Milica>>

#include "config/ServerBlock.hpp"

ServerBlock::ServerBlock() :
	_listen(),
	_host(""),
	_clientMaxBodySize(0),
	_hasCustomErrorPages(false),
	_defaultErrorDir("/default_errors")
{
	_defaultErrorPages.emplace_back(400, _defaultErrorDir + "/400.html");
	_defaultErrorPages.emplace_back(403, _defaultErrorDir + "/403.html");
	_defaultErrorPages.emplace_back(404, _defaultErrorDir + "/404.html");
	_defaultErrorPages.emplace_back(405, _defaultErrorDir + "/405.html");
	_defaultErrorPages.emplace_back(408, _defaultErrorDir + "/408.html");
	_defaultErrorPages.emplace_back(409, _defaultErrorDir + "/409.html");
	_defaultErrorPages.emplace_back(411, _defaultErrorDir + "/411.html");
	_defaultErrorPages.emplace_back(413, _defaultErrorDir + "/413.html");
	_defaultErrorPages.emplace_back(414, _defaultErrorDir + "/414.html");
	_defaultErrorPages.emplace_back(431, _defaultErrorDir + "/431.html");
	_defaultErrorPages.emplace_back(500, _defaultErrorDir + "/500.html");
	_defaultErrorPages.emplace_back(501, _defaultErrorDir + "/501.html");
	_defaultErrorPages.emplace_back(503, _defaultErrorDir + "/503.html");
	_defaultErrorPages.emplace_back(505, _defaultErrorDir + "/505.html");
}

ServerBlock::~ServerBlock() {
}

ServerBlock::ServerBlock(const ServerBlock& other){
	_listen = other._listen;
	_host = other._host;
	_serverName = other._serverName;
	_root = other._root;
	_clientMaxBodySize = other._clientMaxBodySize;
	_errorPages = other._errorPages;
	_defaultErrorPages = other._defaultErrorPages;
	_index = other._index;
	_locationBlocks = other._locationBlocks;
	_hasCustomErrorPages = other._hasCustomErrorPages;
	_defaultErrorDir = other._defaultErrorDir;
	_allowedMethods = other._allowedMethods;
}

ServerBlock& ServerBlock::operator=(const ServerBlock& other){
	if (this != &other){
	_listen = other._listen;
	_host = other._host;
	_serverName = other._serverName;
	_root = other._root;
	_clientMaxBodySize = other._clientMaxBodySize;
	_errorPages = other._errorPages;
	_defaultErrorPages = other._defaultErrorPages;
	_index = other._index;
	_locationBlocks = other._locationBlocks;
	_hasCustomErrorPages = other._hasCustomErrorPages;
	_defaultErrorDir = other._defaultErrorDir;
	_allowedMethods = other._allowedMethods;}
	return (*this);
}

void ServerBlock::clear() {
	_listen.clear();
	_host = "";
	_serverName = "";
	_root = "";
	_clientMaxBodySize = 0;
	_errorPages.clear();
	_locationBlocks.clear();
	_index = "";
	_hasCustomErrorPages = false;
	_defaultErrorDir = "/default_errors";

	_defaultErrorPages.clear();
	_defaultErrorPages.emplace_back(400, _defaultErrorDir + "/400.html");
	_defaultErrorPages.emplace_back(403, _defaultErrorDir + "/403.html");
	_defaultErrorPages.emplace_back(404, _defaultErrorDir + "/404.html");
	_defaultErrorPages.emplace_back(405, _defaultErrorDir + "/405.html");
	_defaultErrorPages.emplace_back(408, _defaultErrorDir + "/408.html");
	_defaultErrorPages.emplace_back(409, _defaultErrorDir + "/409.html");
	_defaultErrorPages.emplace_back(411, _defaultErrorDir + "/411.html");
	_defaultErrorPages.emplace_back(413, _defaultErrorDir + "/413.html");
	_defaultErrorPages.emplace_back(414, _defaultErrorDir + "/414.html");
	_defaultErrorPages.emplace_back(431, _defaultErrorDir + "/431.html");
	_defaultErrorPages.emplace_back(500, _defaultErrorDir + "/500.html");
	_defaultErrorPages.emplace_back(501, _defaultErrorDir + "/501.html");
	_defaultErrorPages.emplace_back(503, _defaultErrorDir + "/503.html");
	_defaultErrorPages.emplace_back(505, _defaultErrorDir + "/505.html");
}

bool ServerBlock::hasLocationBlock(const std::string& path) const {
	return _locationBlocks.find(path) != _locationBlocks.end();
}

LocationBlock ServerBlock::getLocationBlock(const std::string& path) const {
	std::map<std::string, LocationBlock>::const_iterator it = _locationBlocks.find(path);
	if (it != _locationBlocks.end()) {
		return it->second;
	}
	//properly initialize empty block
	LocationBlock emptyBlock;
	emptyBlock.clear();
	return emptyBlock;
}

LocationBlock& ServerBlock::getLocationBlockRef(const std::string& path) {
	std::map<std::string, LocationBlock>::iterator it = _locationBlocks.find(path);
	if (it == _locationBlocks.end()) {
		// If the location block doesn't exist, create a new one
		LocationBlock newBlock;
		_locationBlocks[path] = newBlock;
	}
	// Return a reference to the location block
	return _locationBlocks[path];
}

void ServerBlock::addLocationBlock(const std::string& path, const LocationBlock& block) {
	if (path.length() > MAX_PATH_LENGTH) {
		throw ErrorInvalidConfig("Location path too long (max " + std::to_string(MAX_PATH_LENGTH) + " characters)");
	}
	if (hasLocationBlock(path)) {
		throw ErrorInvalidConfig("Duplicate location block for path: " + path);
	}
	_locationBlocks[path] = block;
}

void ServerBlock::addErrorPage(int status, const std::string& path) {
	if (path.length() > MAX_PATH_LENGTH) {
		throw ErrorInvalidConfig("Error page path too long (max " + std::to_string(MAX_PATH_LENGTH) + " characters)");
	}
	// Check if we already have an error page for this status
	for (size_t i = 0; i < _errorPages.size(); i++) {
		if (_errorPages[i].first == status) {
			throw ErrorInvalidConfig("Duplicate error page for status code: " + std::to_string(status));
		}
	}
	_errorPages.push_back(std::make_pair(status, path));
	_hasCustomErrorPages = true;
}

bool ServerBlock::hasErrorPage(int status) const {
	for (size_t i = 0; i < _errorPages.size(); i++) {
		if (_errorPages[i].first == status) {
			return true;
		}
	}
	return false;
}

std::string ServerBlock::getErrorPage(int status) const {
	if (_hasCustomErrorPages) {
		for (const auto& page : _errorPages) {
			if (page.first == status) {
				return page.second;
			}
		}
	}
	for (const auto& page : _defaultErrorPages) {
		if (page.first == status) {
			return page.second;
		}
	}
	//use default if no custom or default page
	std::string defaultPath = _defaultErrorDir + "/" + std::to_string(status) + ".html";
	return defaultPath;
}

std::vector<std::pair<int, std::string>> ServerBlock::getErrorPages() const {
	return _errorPages;
}

const std::vector<std::pair<int, std::string>>& ServerBlock::getDefaultErrorPages() const {
	return _defaultErrorPages;
}

std::string ServerBlock::getDefaultErrorDir() const {
	return _defaultErrorDir;
}

bool ServerBlock::hasCustomErrorPages() const {
	return _hasCustomErrorPages;
}

bool ServerBlock::hasAllowedMethods() const {
	return _allowedMethods != 0;
}

uint8_t ServerBlock::getAllowedMethods() const {
	return _allowedMethods;
}

void ServerBlock::setAllowedMethods(uint8_t methods) {
	this->_allowedMethods = methods;
}

std::string ServerBlock::allowedMethodsToString() const {
	std::stringstream ss;
		
	if (isMethodAllowed(GET))
		ss << "GET ";
	if (isMethodAllowed(POST))
		ss << "POST ";
	if (isMethodAllowed(DELETE))
		ss << "DELETE ";
	
	std::string result = ss.str();
	if (!result.empty() && result[result.length() - 1] == ' ')
		result = result.substr(0, result.length() - 1);
		
	return result;
}

bool ServerBlock::isMethodAllowed(HttpMethod method) const {
	return (_allowedMethods & method) != 0;
}


const std::vector<std::string>& ServerBlock::getListen() const {
	return _listen;
}

void ServerBlock::addListen(const std::string& port){
	_listen.push_back(port);
}

bool ServerBlock::hasPort(const std::string& port)const {
	return std::find(_listen.begin(), _listen.end(), port) != _listen.end();
}

bool ServerBlock::hasHost() const {
	return !_host.empty();
}

std::string ServerBlock::getHost() const {
	return _host;
}

void ServerBlock::setHost(const std::string& host) {
	this->_host = host;
}

bool ServerBlock::hasServerName() const {
	return !_serverName.empty();
}

std::string ServerBlock::getServerName() const {
	return _serverName;
}

void ServerBlock::setServerName(const std::string& serverName) {
	this->_serverName = serverName;
}

bool ServerBlock::hasRoot() const {
	return !_root.empty();
}

std::string ServerBlock::getRoot() const {
	return _root;
}

void ServerBlock::setRoot(const std::string& root) {
	this->_root = root;
}

bool ServerBlock::hasClientMaxBodySize() const {
	return _clientMaxBodySize > 0;
}

size_t ServerBlock::getClientMaxBodySize() const {
	return _clientMaxBodySize;
}

void ServerBlock::setClientMaxBodySize(size_t size) {
	this->_clientMaxBodySize = size;
}

bool ServerBlock::hasIndex() const {
	return !_index.empty();
}

std::string ServerBlock::getIndex() const {
	return _index;
}

void ServerBlock::setIndex(const std::string& index) {
	this->_index = index;
}

std::map<std::string, LocationBlock> ServerBlock::getLocationBlocks() const {
	return _locationBlocks;
}
