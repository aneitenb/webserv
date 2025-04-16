// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<ConfigErrors.cpp>> -- <<Aida, Ilmari, Milica>>

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

// ===== ErrorNoMatchingServer =====
ErrorNoMatchingServer::ErrorNoMatchingServer(const std::string& message) 
    : ConfigError(message) {}

std::string ErrorNoMatchingServer::getErrorType() const {
    return "Server Matching Error";
}
