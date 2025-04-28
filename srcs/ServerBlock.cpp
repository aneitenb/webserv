#include "../includes/ServerBlock.hpp"

ServerBlock::ServerBlock() :
	listen(""),
	host(""),
	client_max_body_size(0) {
}

ServerBlock::~ServerBlock() {
}

void ServerBlock::clear() {
	listen = "";
	host = "";
	server_name = "";
	root = "";
	client_max_body_size = 0;
	error_pages.clear();
	location_blocks.clear();
	index = "";
}

bool ServerBlock::hasLocationBlock(const std::string& path) const {
	return location_blocks.find(path) != location_blocks.end();
}

LocationBlock ServerBlock::getLocationBlock(const std::string& path) const {
	std::map<std::string, LocationBlock>::const_iterator it = location_blocks.find(path);
	if (it != location_blocks.end()) {
		return it->second;
	}
	//properly initiailze empty block
	LocationBlock emptyBlock;
	emptyBlock.clear();
	return emptyBlock;
}

void ServerBlock::addLocationBlock(const std::string& path, const LocationBlock& block) {
	if (path.length() > MAX_ROOT_PATH_LENGTH) {
		throw std::runtime_error("Location path too long (max " + std::to_string(MAX_ROOT_PATH_LENGTH) + " characters)");
	}
	
	// Check if the path already exists
	// if (hasLocationBlock(path)) {
	// 	throw std::runtime_error("Duplicate location block for path: " + path);
	// }
	
	// location_blocks[path] = block;

	// Use insert or update approach for the map
	std::pair<std::map<std::string, LocationBlock>::iterator, bool> result = 
		location_blocks.insert(std::make_pair(path, block));
	
	if (!result.second) {
		// Path already exists, update instead
		result.first->second = block;
	}
}

void ServerBlock::addErrorPage(int status, const std::string& path) {
	// Check if error page path is too long
	if (path.length() > MAX_ROOT_PATH_LENGTH) {
		throw std::runtime_error("Error page path too long (max " + std::to_string(MAX_ROOT_PATH_LENGTH) + " characters)");
	}
	
	// Check if we already have an error page for this status
	for (size_t i = 0; i < error_pages.size(); i++) {
		if (error_pages[i].first == status) {
			throw std::runtime_error("Duplicate error page for status code: " + std::to_string(status));
		}
	}
	
	error_pages.push_back(std::make_pair(status, path));
}

bool ServerBlock::hasErrorPage(int status) const {
	for (size_t i = 0; i < error_pages.size(); i++) {
		if (error_pages[i].first == status) {
			return true;
		}
	}
	return false;
}

std::string ServerBlock::getErrorPage(int status) const {
	for (size_t i = 0; i < error_pages.size(); i++) {
		if (error_pages[i].first == status) {
			return error_pages[i].second;
		}
	}
	return "";
}

std::string ServerBlock::getListen() const {
	return listen;
}

void ServerBlock::setListen(const std::string& listen) {
	this->listen = listen;
}

std::string ServerBlock::getHost() const {
	return host;
}

void ServerBlock::setHost(const std::string& host) {
	this->host = host;
}

std::string ServerBlock::getServerName() const {
	return server_name;
}

void ServerBlock::setServerName(const std::string& server_name) {
	this->server_name = server_name;
}

std::string ServerBlock::getRoot() const {
	return root;
}

void ServerBlock::setRoot(const std::string& root) {
	this->root = root;
}

size_t ServerBlock::getClientMaxBodySize() const {
	return client_max_body_size;
}

void ServerBlock::setClientMaxBodySize(size_t size) {
	this->client_max_body_size = size;
}

std::vector<std::pair<int, std::string>> ServerBlock::getErrorPages() const {
	return error_pages;
}

std::string ServerBlock::getIndex() const {
	return index;
}

void ServerBlock::setIndex(const std::string& index) {
	this->index = index;
}

std::map<std::string, LocationBlock> ServerBlock::getLocationBlocks() const {
	return location_blocks;
}