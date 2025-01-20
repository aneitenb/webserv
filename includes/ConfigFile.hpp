/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigFile.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aneitenb <aneitenb@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/20 12:47:33 by aneitenb          #+#    #+#             */
/*   Updated: 2025/01/20 15:56:07 by aneitenb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Webserv.hpp"

//Type definitions for readability
typedef std::map<std::string, std::string> ServerConfig;
typedef std::vector<ServerConfig> ServerConfigs;

class ConfigFile
{
	private:
	std::ifstream _configFile;
	std::string _configContent;
	ServerConfigs _servers;		//Stores all server configurations
	std::vector<int> _ports;	//Stores all server ports

	//Private member functions
	bool parseFile(const std::string& filename);
	bool parseServers();
	bool validateConfig();
	bool checkPorts();	//check for duplicate ports
	
	///Utility functions
	bool isValidPort(const std::string& port);
	void trimString(std::string& str);

	public:
	ConfigFile();
    ~ConfigFile();

    // Delete copy constructor and assignment operator
    ConfigFile(const ConfigFile&) = delete;
    ConfigFile& operator=(const ConfigFile&) = delete;

    // Main function to initialize and parse config
    bool init(const std::string& filename);

    // Getters
    const ServerConfigs& getServers() const;
    const std::vector<int>& getPorts() const;

    // Custom exceptions
    class ConfigFileError : public std::runtime_error {
        public:
            explicit ConfigFileError(const std::string& msg) : std::runtime_error(msg) {}
    };
}