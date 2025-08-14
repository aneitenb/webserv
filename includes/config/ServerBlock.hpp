// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<ServerBlock.hpp>> -- <<Aida, Ilmari, Milica>>

#pragma once

#include "ConfigErrors.hpp"
#include "LocationBlock.hpp"

#define MAX_SERVER_NAME_LENGTH 50
#define MAX_PATH_LENGTH 50
#define MAX_ROOT_PATH_LENGTH 255
#define MAX_BODY_SIZE 104857601	//100MB

#ifndef CLIENT_DEFAULT_TIMEOUT
# define CLIENT_DEFAULT_TIMEOUT	30000U // ms (30s)
#endif

class ServerBlock {
private:
	std::vector<std::string> _listen;
	std::string _host;
	std::string _serverName;
	std::string _root;
	size_t _clientMaxBodySize;
	bool _autoindex;
	bool _autoindexSet;
	std::vector<std::pair<int, std::string>> _errorPages;
	std::string _index;
	std::map<std::string, LocationBlock> _locationBlocks;
	bool _hasCustomErrorPages;
	bool _maxBodySizeSet;
	uint8_t _allowedMethods;
	std::string _upload_store;
	u64		_timeout;

public:
	ServerBlock();
	~ServerBlock();
	ServerBlock(const ServerBlock& other);
	ServerBlock& operator=(const ServerBlock& other);
	bool operator==(const ServerBlock& other) const;

	void clear();
	
	// Location block utility functions
	bool hasLocationBlock(const std::string& path) const;
	LocationBlock getLocationBlock(const std::string& path) const;
	LocationBlock& getLocationBlockRef(const std::string& path);
	void addLocationBlock(const std::string& path, const LocationBlock& block);
	
	// Error page management
	void addErrorPage(int status, const std::string& path);
	bool hasErrorPage(int status) const;
	std::string getErrorPage(int status) const;
	std::vector<std::pair<int, std::string>> getErrorPages() const;
	bool hasCustomErrorPages() const;
	
	// Getter and setter methods
	bool hasUploadStore() const;
	std::string getUploadStore() const;
	void setUploadStore(const std::string& uploadStore);

	const std::vector<std::string>& getListen() const;
	void addListen(const std::string& port);
	bool hasPort(const std::string& port) const;

	bool hasAllowedMethods() const;
	uint8_t getAllowedMethods() const;
	void setAllowedMethods(uint8_t methods);
	std::string allowedMethodsToString() const;
	bool isMethodAllowed(HttpMethod method) const;
	
	bool hasHost() const;
	std::string getHost() const;
	void setHost(const std::string& host);
	
	bool hasServerName() const;
	std::string getServerName() const;
	void setServerName(const std::string& server_name);
	
	bool hasRoot() const;
	std::string getRoot() const;
	void setRoot(const std::string& root);
	
	bool hasClientMaxBodySize() const;
	size_t getClientMaxBodySize() const;
	void setClientMaxBodySize(size_t size);
	
	bool hasIndex() const;
	std::string getIndex() const;
	void setIndex(const std::string& index);

	bool hasAutoindex() const;
	bool getAutoindex() const;
	void setAutoindex(bool value);
	
	std::map<std::string, LocationBlock> getLocationBlocks() const;
};
