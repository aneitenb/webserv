/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigErrors.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aneitenb <aneitenb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/29 15:46:34 by aneitenb          #+#    #+#             */
/*   Updated: 2025/03/31 14:08:04 by aneitenb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config/ConfigErrors.hpp"

// ===== ConfigError Base Class =====
ConfigError::ConfigError(const std::string& message) : _message(message) {}

ConfigError::~ConfigError() throw() {}

const char* ConfigError::what() const throw() {
	return _message.c_str();
}

std::string ConfigError::getErrorType() const {
	return "Configuration Error";
}

// ===== ErrorOpeningConfFile =====
ErrorOpeningConfFile::ErrorOpeningConfFile(const std::string& message) 
	: ConfigError(message) {}

std::string ErrorOpeningConfFile::getErrorType() const {
	return "File Error";
}

// ===== ErrorInvalidConfig =====
ErrorInvalidConfig::ErrorInvalidConfig(const std::string& message) 
	: ConfigError(message) {}

std::string ErrorInvalidConfig::getErrorType() const {
	return "Config Format Error";
}

// ===== ErrorInvalidPort =====
ErrorInvalidPort::ErrorInvalidPort(const std::string& message) 
	: ConfigError(message) {}

std::string ErrorInvalidPort::getErrorType() const {
	return "Port Error";
}

// ===== ErrorInvalidIP =====
ErrorInvalidIP::ErrorInvalidIP(const std::string& message) 
	: ConfigError(message) {}

std::string ErrorInvalidIP::getErrorType() const {
	return "IP Address Error";
}
