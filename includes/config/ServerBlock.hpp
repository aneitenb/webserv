/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerBlock.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/31 12:48:30 by aneitenb          #+#    #+#             */
/*   Updated: 2025/04/06 21:39:02 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "WebServer.hpp"
// #include "LocationBlock.hpp"

#define MAX_SERVER_NAME_LENGTH 50
#define MAX_PATH_LENGTH 50
#define MAX_ROOT_PATH_LENGTH 255
#define MAX_BODY_SIZE 1073741824 // 1GB

class ServerBlock {
private:
	std::vector<std::string> _listen;
	std::string _host;
	std::string _serverName;
	std::string _root;
	size_t _clientMaxBodySize;
	std::vector<std::pair<int, std::string>> _errorPages;
	std::string _index;
	// std::map<std::string, LocationBlock> _locationBlocks;
	bool _hasCustomErrorPages;
	std::string _defaultErrorDir;

public:
	ServerBlock();
	~ServerBlock();
	//I need a copy constructor/copy assignment operator for VirtualHost objects
	ServerBlock(const ServerBlock& object);
	ServerBlock& operator=(const ServerBlock& object);

	void clear();
	
	// Location block utility functions
	// bool hasLocationBlock(const std::string& path) const;
	// LocationBlock getLocationBlock(const std::string& path) const;
	// void addLocationBlock(const std::string& path, const LocationBlock& block);
	
	// Error page management
	void addErrorPage(int status, const std::string& path);
	bool hasErrorPage(int status) const;
	std::string getErrorPage(int status) const;
	std::vector<std::pair<int, std::string>> getErrorPages() const;
	std::string getDefaultErrorDir() const;
	bool hasCustomErrorPages() const;
	
	// Getter and setter methods
	const std::vector<std::string>& getListen() const;
	void addListen(const std::string& port);
	bool hasPort(const std::string& port) const;
	//void setListen(const std::string& listen);
	
	std::string getHost() const;
	void setHost(const std::string& host);
	
	std::string getServerName() const;
	void setServerName(const std::string& server_name);
	
	std::string getRoot() const;
	void setRoot(const std::string& root);
	
	size_t getClientMaxBodySize() const;
	void setClientMaxBodySize(size_t size);
	
	
	std::string getIndex() const;
	void setIndex(const std::string& index);
	
	// std::map<std::string, LocationBlock> getLocationBlocks() const;
};