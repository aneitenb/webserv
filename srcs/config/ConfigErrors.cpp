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
	return "Configuration error";
}

// ===== ErrorOpeningConfFile =====
ErrorOpeningConfFile::ErrorOpeningConfFile(const std::string& message) 
	: ConfigError(message) {}

std::string ErrorOpeningConfFile::getErrorType() const {
	return "File error";
}

// ===== ErrorInvalidConfig =====
ErrorInvalidConfig::ErrorInvalidConfig(const std::string& message) 
	: ConfigError(message) {}

std::string ErrorInvalidConfig::getErrorType() const {
	return "Config format error";
}

// ===== ErrorInvalidPort =====
ErrorInvalidPort::ErrorInvalidPort(const std::string& message) 
	: ConfigError(message) {}

std::string ErrorInvalidPort::getErrorType() const {
	return "Port error";
}

// ===== ErrorInvalidIP =====
ErrorInvalidIP::ErrorInvalidIP(const std::string& message) 
	: ConfigError(message) {}

std::string ErrorInvalidIP::getErrorType() const {
	return "IP address error";
}

// ===== ErrorNoMatchingServer =====
ErrorNoMatchingServer::ErrorNoMatchingServer(const std::string& message) 
    : ConfigError(message) {}

std::string ErrorNoMatchingServer::getErrorType() const {
    return "Server matching error";
}
