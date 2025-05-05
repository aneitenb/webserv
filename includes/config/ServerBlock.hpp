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
#define MAX_BODY_SIZE 1073741824 // 1GB

class ServerBlock {
private:
	std::vector<std::string> _listen;
	std::string _host;
	std::string _serverName;
	std::string _root;
	size_t _clientMaxBodySize;
	std::vector<std::pair<int, std::string>> _errorPages;
	std::vector<std::pair<int, std::string>> _defaultErrorPages;
	std::string _index;
	std::map<std::string, LocationBlock> _locationBlocks;
	bool _hasCustomErrorPages;
	std::string _defaultErrorDir;
	uint8_t _allowedMethods;

public:
	ServerBlock();
	~ServerBlock();
	ServerBlock(const ServerBlock& other);
	ServerBlock& operator=(const ServerBlock& other);

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
	const std::vector<std::pair<int, std::string>>& getDefaultErrorPages() const;
	std::string getDefaultErrorDir() const;
	bool hasCustomErrorPages() const;
	
	// Getter and setter methods
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
	
	std::map<std::string, LocationBlock> getLocationBlocks() const;
};
