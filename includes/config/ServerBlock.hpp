/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerBlock.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aneitenb <aneitenb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/31 12:48:30 by aneitenb          #+#    #+#             */
/*   Updated: 2025/03/31 12:50:10 by aneitenb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Webserv.hpp"
#include "LocationBlock.hpp"

#define MAX_SERVER_NAME_LENGTH 50
#define MAX_ROOT_PATH_LENGTH 255
#define MAX_BODY_SIZE 1073741824 // 1GB

class ServerBlock {
private:
	std::string listen;
	std::string host;
	std::string server_name;
	std::string root;
	size_t client_max_body_size;
	std::vector<std::pair<int, std::string>> error_pages;
	std::string index;
	std::map<std::string, LocationBlock> location_blocks;

public:
	ServerBlock();
	~ServerBlock();

	void clear();
	
	// Location block utility functions
	bool hasLocationBlock(const std::string& path) const;
	LocationBlock getLocationBlock(const std::string& path) const;
	void addLocationBlock(const std::string& path, const LocationBlock& block);
	
	// Error page management
	void addErrorPage(int status, const std::string& path);
	bool hasErrorPage(int status) const;
	std::string getErrorPage(int status) const;
	
	// Getter and setter methods
	std::string getListen() const;
	void setListen(const std::string& listen);
	
	std::string getHost() const;
	void setHost(const std::string& host);
	
	std::string getServerName() const;
	void setServerName(const std::string& server_name);
	
	std::string getRoot() const;
	void setRoot(const std::string& root);
	
	size_t getClientMaxBodySize() const;
	void setClientMaxBodySize(size_t size);
	
	std::vector<std::pair<int, std::string>> getErrorPages() const;
	
	std::string getIndex() const;
	void setIndex(const std::string& index);
	
	std::map<std::string, LocationBlock> getLocationBlocks() const;
};
