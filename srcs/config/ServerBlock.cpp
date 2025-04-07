/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerBlock.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/31 12:48:34 by aneitenb          #+#    #+#             */
/*   Updated: 2025/04/07 15:40:50 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config/ServerBlock.hpp"
#include <stdexcept>
#include <algorithm>

ServerBlock::ServerBlock() :
	_listen(),
	_host(""),
	_clientMaxBodySize(0),
	_index("index.html"),
	_hasCustomErrorPages(false),
	_defaultErrorDir("/errors")
{
}

ServerBlock::~ServerBlock() {
}


//check with Aida if this seems fine with her
ServerBlock::ServerBlock(const ServerBlock& object){
	_listen = object._listen;
	_host = object._host;
	_clientMaxBodySize = object._clientMaxBodySize;
	_index = object._index;
	_hasCustomErrorPages = object._hasCustomErrorPages;
	_defaultErrorDir = object._defaultErrorDir;
	_serverName = object._serverName;
	_errorPages = object._errorPages;
	_root = object._root;
	//_locationBlocks = object._locationBlocks;
}

ServerBlock& ServerBlock::operator=(const ServerBlock& object){
	if (this != &object){
		_listen = object._listen;
		_host = object._host;
		_clientMaxBodySize = object._clientMaxBodySize;
		_index = object._index;
		_hasCustomErrorPages = object._hasCustomErrorPages;
		_defaultErrorDir = object._defaultErrorDir;
		_serverName = object._serverName;
		_errorPages = object._errorPages;
		_root = object._root;
		//_locationBlocks = object._locationBlocks;		
	}
	return (*this);
}

void ServerBlock::clear() {
	_listen.clear();
	_host = "";
	_serverName = "";
	_root = "";
	_clientMaxBodySize = 0;
	_errorPages.clear();
	// _locationBlocks.clear();
	_index = "index.html";
	_hasCustomErrorPages = false;
    _defaultErrorDir = "/errors";
}

// bool ServerBlock::hasLocationBlock(const std::string& path) const {
// 	return _locationBlocks.find(path) != _locationBlocks.end();
// }

// LocationBlock ServerBlock::getLocationBlock(const std::string& path) const {
// 	std::map<std::string, LocationBlock>::const_iterator it = _locationBlocks.find(path);
// 	if (it != _locationBlocks.end()) {
// 		return it->second;
// 	}
// 	//properly initiailze empty block
// 	LocationBlock emptyBlock;
// 	emptyBlock.clear();
// 	return emptyBlock;
// }

// void ServerBlock::addLocationBlock(const std::string& path, const LocationBlock& block) {
// 	if (path.length() > MAX_PATH_LENGTH) {
// 		throw std::runtime_error("Location path too long (max " + std::to_string(MAX_PATH_LENGTH) + " characters)");
// 	}
// 	if (hasLocationBlock(path)) {
// 		throw std::runtime_error("Duplicate location block for path: " + path);
// 	}
// 	_locationBlocks[path] = block;
// 	}
// }

void ServerBlock::addErrorPage(int status, const std::string& path) {
	if (path.length() > MAX_PATH_LENGTH) {
		throw std::runtime_error("Error page path too long (max " + std::to_string(MAX_PATH_LENGTH) + " characters)");
	}
	// Check if we already have an error page for this status
	for (size_t i = 0; i < _errorPages.size(); i++) {
		if (_errorPages[i].first == status) {
			throw std::runtime_error("Duplicate error page for status code: " + std::to_string(status));
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
	//use default if no custom page
	std::string defaultPath = _defaultErrorDir + "/" + std::to_string(status) + ".html";
	return defaultPath;
}

std::vector<std::pair<int, std::string>> ServerBlock::getErrorPages() const {
	return _errorPages;
}

std::string ServerBlock::getDefaultErrorDir() const {
	return _defaultErrorDir;
}

bool ServerBlock::hasCustomErrorPages() const {
	return _hasCustomErrorPages;
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

std::string ServerBlock::getHost() const {
	return _host;
}

void ServerBlock::setHost(const std::string& host) {
	this->_host = host;
}

std::string ServerBlock::getServerName() const {
	return _serverName;
}

void ServerBlock::setServerName(const std::string& serverName) {
	this->_serverName = serverName;
}

std::string ServerBlock::getRoot() const {
	return _root;
}

void ServerBlock::setRoot(const std::string& root) {
	this->_root = root;
}

size_t ServerBlock::getClientMaxBodySize() const {
	return _clientMaxBodySize;
}

void ServerBlock::setClientMaxBodySize(size_t size) {
	this->_clientMaxBodySize = size;
}

std::string ServerBlock::getIndex() const {
	return _index;
}

void ServerBlock::setIndex(const std::string& index) {
	this->_index = index;
}

// std::map<std::string, LocationBlock> ServerBlock::getLocationBlocks() const {
// 	return _locationBlocks;
// }