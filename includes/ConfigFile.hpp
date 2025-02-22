/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigFile.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aneitenb <aneitenb@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/20 12:47:33 by aneitenb          #+#    #+#             */
/*   Updated: 2025/02/11 17:15:04 by aneitenb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Webserv.hpp"

//Type definitions for readability
typedef std::map<std::string, std::string> ServerBlocks;

class ConfigurationFile {
private:
	std::ifstream				_configFile;
	std::string					_configContent;
	std::vector<ServerBlocks>	_servers;
	std::vector<size_t>			_ports;
	
	// FIle handling
	int	_readFile(void);
	int	_parseConfigFile(void);
	void	_setupDefaultServer(void);
	
	//validation methods & helper functions
	bool _isValidPort(const std::string& port) const;  // Just validation
	int _addPort(const std::string& port);  
	bool _isValidIP(const std::string& ip) const;
	bool _isValidPath(const std::string& path) const;
	bool _isAutoindexValid(const std::string& value) const;
	bool _validateServerBlock(const ServerBlocks& directives) const;
	bool _isValidDirective(const std::string& directive) const;
	bool _checkPermissions(const std::string& path, bool needWrite = false) const;
	std::string _trimWhitespace(const std::string& str) const;
	static const std::set<std::string> _validDirectives;

	//Management
	std::string _getValue(const ServerBlocks& directives, const std::string& key) const;
	void _setValue(ServerBlocks& directives, const std::string& key, const std::string& value);
	bool _hasValue(const ServerBlocks& directives, const std::string& key) const;

	//Delete copy operators to avoid two objects managing a single file
	ConfigurationFile(ConfigurationFile const &rhs);
	ConfigurationFile& operator=(ConfigurationFile const &rhs);
	
public:
	ConfigurationFile(void);
	~ConfigurationFile(void);

	// Public member functions
	void								initializeConfFile(const std::string& filename);
	const std::vector<ServerBlocks>&	getServers(void) const;
	const std::vector<size_t>&			getPorts(void) const;
};
