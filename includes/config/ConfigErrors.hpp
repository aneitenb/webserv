/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigErrors.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aneitenb <aneitenb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/29 15:43:47 by aneitenb          #+#    #+#             */
/*   Updated: 2025/04/02 17:21:34 by aneitenb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "../Webserv.hpp"

// base class for all configuration errors
class ConfigError : public std::exception {
private:
	std::string _message;

public:
	ConfigError(const std::string& message = "Configuration error");
	virtual ~ConfigError() throw();
	
	virtual const char* what() const throw();
	virtual std::string getErrorType() const;
};

class ErrorOpeningConfFile : public ConfigError {
public:
	ErrorOpeningConfFile(const std::string& message = "Could not open configuration file");
	std::string getErrorType() const override;
};

class ErrorInvalidConfig : public ConfigError {
public:
	ErrorInvalidConfig(const std::string& message = "Invalid configuration format");
	std::string getErrorType() const override;
};

class ErrorInvalidPort : public ConfigError {
public:
	ErrorInvalidPort(const std::string& message = "Invalid port number");
	std::string getErrorType() const override;
};

class ErrorInvalidIP : public ConfigError {
public:
	ErrorInvalidIP(const std::string& message = "Invalid IP address");
	std::string getErrorType() const override;
};

class ErrorNoMatchingServer : public ConfigError {
public:
	ErrorNoMatchingServer(const std::string& message = "No matching server found for the requested endpoint");
	std::string getErrorType() const override;
};
