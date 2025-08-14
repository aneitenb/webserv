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
	_serverName(""),
	_root(""),
	_clientMaxBodySize(1024 * 1024),	//1 megabyte as default value
	_autoindex(false),
	_autoindexSet(false),
	_index(""),
	_hasCustomErrorPages(false),
	_maxBodySizeSet(false),
	_allowedMethods(0),
	_upload_store(""),
	_timeout(CLIENT_DEFAULT_TIMEOUT)
{}

ServerBlock::~ServerBlock() {
}

ServerBlock::ServerBlock(const ServerBlock& other){
	_listen = other._listen;
	_host = other._host;
	_serverName = other._serverName;
	_root = other._root;
	_clientMaxBodySize = other._clientMaxBodySize;
	_autoindex = other._autoindex;
	_autoindexSet = other._autoindexSet;
	_errorPages = other._errorPages;
	_index = other._index;
	_locationBlocks = other._locationBlocks;
	_hasCustomErrorPages = other._hasCustomErrorPages;
	_maxBodySizeSet = other._maxBodySizeSet;
	_allowedMethods = other._allowedMethods;
	_upload_store = other._upload_store;
	_timeout = other._timeout;
}

ServerBlock& ServerBlock::operator=(const ServerBlock& other){
	if (this != &other){
	_listen = other._listen;
	_host = other._host;
	_serverName = other._serverName;
	_root = other._root;
	_clientMaxBodySize = other._clientMaxBodySize;
	_autoindex = other._autoindex;
	_autoindexSet = other._autoindexSet;
	_errorPages = other._errorPages;
	_index = other._index;
	_locationBlocks = other._locationBlocks;
	_hasCustomErrorPages = other._hasCustomErrorPages;
	_maxBodySizeSet = other._maxBodySizeSet;
	_allowedMethods = other._allowedMethods;
	_upload_store = other._upload_store;
	_timeout = other._timeout;}
	return (*this);
}

bool ServerBlock::operator==(const ServerBlock& other) const{
	if (this->_listen == other._listen \
	&& this->_host == other._host \
	&& this->_serverName == other._serverName \
	&& this->_root == other._root \
	&& this->_clientMaxBodySize == other._clientMaxBodySize \
	&& this->_autoindex == other._autoindex \
	&& this->_autoindexSet == other._autoindexSet \
	&& this->_errorPages == other._errorPages \
	&& this->_index == other._index \
	&& this->_locationBlocks == other._locationBlocks \
	&& this->_hasCustomErrorPages == other._hasCustomErrorPages \
	&& this->_maxBodySizeSet == other._maxBodySizeSet \
	&& this->_allowedMethods == other._allowedMethods \
	&& this->_upload_store == other._upload_store \
	&& this->_timeout == other._timeout)
		return (true);
	return (false);
}

void ServerBlock::clear() {
	_listen.clear();
	_host = "";
	_serverName = "";
	_root = "";
	_clientMaxBodySize = 1024 * 1024;
	_maxBodySizeSet = false;
	_errorPages.clear();
	_locationBlocks.clear();
	_index = "";
	_hasCustomErrorPages = false;
	_allowedMethods = 0;
	_upload_store = "";
	_timeout = CLIENT_DEFAULT_TIMEOUT;
}

bool ServerBlock::hasUploadStore() const {
	return !_upload_store.empty();
}

std::string ServerBlock::getUploadStore() const {
	return _upload_store;
}

void ServerBlock::setUploadStore(const std::string& uploadStore) {
	this->_upload_store = uploadStore;
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
				return (page.second);
			}
		}
	}
	return "";
}

std::vector<std::pair<int, std::string>> ServerBlock::getErrorPages() const {
	return _errorPages;
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
	std::cout << "DEBUG: Adding port '" << port << "' (length: " << port.length() << ")" << std::endl;
	if (port.length() > 10) {
        std::cout << "ERROR: Port string suspiciously long: " << port << std::endl;
        throw std::runtime_error("Invalid port length");
	}
	_listen.push_back(port);
    std::cout << "DEBUG: Vector now has " << _listen.size() << " ports" << std::endl;
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
	return _maxBodySizeSet;
}

size_t ServerBlock::getClientMaxBodySize() const {
	return _clientMaxBodySize;
}

void ServerBlock::setClientMaxBodySize(size_t size) {
	this->_clientMaxBodySize = size;
	this->_maxBodySizeSet = true;
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

bool ServerBlock::hasAutoindex() const{
	return _autoindexSet;
}

bool ServerBlock::getAutoindex() const {
	return _autoindex;
}

void ServerBlock::setAutoindex(bool value) {
	_autoindex = value;
	_autoindexSet = true;
}

std::map<std::string, LocationBlock> ServerBlock::getLocationBlocks() const {
	return _locationBlocks;
}
