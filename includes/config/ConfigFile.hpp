/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigFile.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aneitenb <aneitenb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/20 12:47:33 by aneitenb          #+#    #+#             */
/*   Updated: 2025/04/12 16:21:17 by aneitenb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "Webserv.hpp"
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
	void _parseLocationDirective(ServerBlock& server, LocationBlock& locBlock, 
			const std::string& key, const std::string& value);

	bool _validateServerBlock(const ServerBlock& server) const;
	bool _validateLocationBlock(const std::string& path, const LocationBlock& block) const;
	bool _checkForDuplicateServers() const;

	uint8_t _parseHttpMethods(const std::string& methods);

public:
	// Constructors and destructors
	ConfigurationFile(void);
	~ConfigurationFile(void);

	void initialize(const std::string& filename);

	ServerBlock getServerBlock(size_t index) const;
	const ServerBlock& getServerBlockByIPPort(const std::string& ipAddress, const std::string& port) const;
	std::vector<const ServerBlock*> getAllServerBlocksByIPPort(const std::string& ipAddress, const std::string& port) const;
	const ServerBlock& getServerBlockByHostPortName(const std::string& ipAddress, const std::string& port, 
				const std::string& serverName) const;
	size_t getServerCount() const;
};

