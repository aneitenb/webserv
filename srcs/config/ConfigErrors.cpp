/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigErrors.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aneitenb <aneitenb@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/29 15:46:34 by aneitenb          #+#    #+#             */
/*   Updated: 2025/01/29 16:56:26 by aneitenb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/ConfigErrors.hpp"

/************************************
*		ErrorOpeningConfFile		*
************************************/
ErrorOpeningConfFile::ErrorOpeningConfFile() : _message("Could not open configuration file") {}	//constructor
ErrorOpeningConfFile::ErrorOpeningConfFile(const ErrorOpeningConfFile& other) : _message(other._message) {}	//copy contructor
ErrorOpeningConfFile& ErrorOpeningConfFile::operator=(const ErrorOpeningConfFile& other) {	//copy assignment operator
    if (this != &other) {
        _message = other._message;
    }
    return *this;
}
ErrorOpeningConfFile::~ErrorOpeningConfFile() throw() {}	//destructor

const char* ErrorOpeningConfFile::what() const throw() {
    return _message.c_str();
}

/****************************
*		Invalid Config		*
****************************/
ErrorInvalidConfig::ErrorInvalidConfig(const std::string& msg) : _message(msg) {}
ErrorInvalidConfig::ErrorInvalidConfig(const ErrorInvalidConfig& other) : _message(other._message) {}
ErrorInvalidConfig& ErrorInvalidConfig::operator=(const ErrorInvalidConfig& other) {
    if (this != &other) {
        _message = other._message;
    }
    return *this;
}
ErrorInvalidConfig::~ErrorInvalidConfig() throw() {}

const char* ErrorInvalidConfig::what() const throw() {
    return _message.c_str();
}

/****************************
*		Invalid Port		*
****************************/
ErrorInvalidPort::ErrorInvalidPort(const std::string& msg) : _message(msg) {}
ErrorInvalidPort::ErrorInvalidPort(const ErrorInvalidPort& other) : _message(other._message) {}
ErrorInvalidPort& ErrorInvalidPort::operator=(const ErrorInvalidPort& other) {
    if (this != &other) {
        _message = other._message;
    }
    return *this;
}
ErrorInvalidPort::~ErrorInvalidPort() throw() {}

const char* ErrorInvalidPort::what() const throw() {
    return _message.c_str();
}


/************************
*		Invalid IP		*
************************/
ErrorInvalidIP::ErrorInvalidIP(const std::string& msg) : _message(msg) {}
ErrorInvalidIP::ErrorInvalidIP(const ErrorInvalidIP& other) : _message(other._message) {}
ErrorInvalidIP& ErrorInvalidIP::operator=(const ErrorInvalidIP& other) {
    if (this != &other) {
        _message = other._message;
    }
    return *this;
}
ErrorInvalidIP::~ErrorInvalidIP() throw() {}

const char* ErrorInvalidIP::what() const throw() {
    return _message.c_str();
}
