// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<ConfigFile.hpp>> -- <<Aida, Ilmari, Milica>>

#pragma once

#include "LocationBlock.hpp"
#include "ServerBlock.hpp"

class ConfigError;

class ConfigurationFile {
private:
	std::ifstream _configFile;
	std::string _configContent;

	std::vector<ServerBlock> _servers;

	static const std::set<std::string> _serverOnlyDirectives;
	static const std::set<std::string> _locationOnlyDirectives;
	static const std::set<std::string> _commonDirectives;

	int _readFile(void);
	int _parseConfigFile(void);

	std::string _trimWhitespace(const std::string& str) const;
	bool _isValidIP(const std::string& ip) const;
	bool _isValidPort(const std::string& port) const;
	bool _isValidPathFormat(const std::string& path) const;
	bool _isValidLocationPath(const std::string& path) const;
	bool _isValidFilenameFormat(const std::string& filename) const;
	bool _isValidServerDirective(const std::string& directive) const;
	bool _isValidLocationDirective(const std::string& directive) const;
	bool _isValidHostname(const std::string& hostname) const;
	
	void _parseServerDirective(ServerBlock& server, const std::string& key, const std::string& value);
	void _parseLocationDirective(LocationBlock& locBlock, const std::string& key, const std::string& value);

	bool _validateServerBlock(ServerBlock& server) const;
	bool _validateLocationBlock(const std::string& path, const LocationBlock& block) const;
	bool _checkForDuplicateServers() const;

	uint8_t _parseHttpMethods(const std::string& methods);

public:
	// Constructors and destructors
	ConfigurationFile(void);
	~ConfigurationFile(void);

	void initialize(const std::string& filename);

	const ServerBlock& getServerBlock(size_t index) const;
	const ServerBlock& getServerBlockByIPPort(const std::string& ipAddress, const std::string& port) const;
	std::vector<const ServerBlock*> getAllServerBlocksByIPPort(const std::string& ipAddress, const std::string& port) const;
	const ServerBlock& getServerBlockByHostPortName(const std::string& ipAddress, const std::string& port, 
				const std::string& serverName) const;
	size_t getServerCount() const;
	//get all server blocks func needed for webserv initialising
	std::vector<ServerBlock>& getAllServerBlocks();
};

