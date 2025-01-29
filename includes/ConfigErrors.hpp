/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigErrors.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aneitenb <aneitenb@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/29 15:43:47 by aneitenb          #+#    #+#             */
/*   Updated: 2025/01/29 16:55:16 by aneitenb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Webserv.hpp"

class ErrorOpeningConfFile : public std::exception {
private:
	std::string _message;
public:
	explicit ErrorOpeningConfFile();	/// explicit prevents implicit type conversions
	ErrorOpeningConfFile(const ErrorOpeningConfFile& other);
	ErrorOpeningConfFile& operator=(const ErrorOpeningConfFile& other);
	virtual ~ErrorOpeningConfFile() throw();
	virtual const char* what() const throw();
};

class ErrorInvalidConfig : public std::exception {
private:
	std::string _message;
public:
	explicit ErrorInvalidConfig(const std::string& msg);
	ErrorInvalidConfig(const ErrorInvalidConfig& other);
	ErrorInvalidConfig& operator=(const ErrorInvalidConfig& other);
	virtual ~ErrorInvalidConfig() throw();
	virtual const char* what() const throw();
};

class ErrorInvalidPort : public std::exception {
private:
	std::string _message;
public:
	explicit ErrorInvalidPort(const std::string& msg);
	ErrorInvalidPort(const ErrorInvalidPort& other);
	ErrorInvalidPort& operator=(const ErrorInvalidPort& other);
	virtual ~ErrorInvalidPort() throw();
	virtual const char* what() const throw();
};

class ErrorInvalidIP : public std::exception {
private:
	std::string _message;
public:
	explicit ErrorInvalidIP(const std::string& msg);
	ErrorInvalidIP(const ErrorInvalidIP& other);
	ErrorInvalidIP& operator=(const ErrorInvalidIP& other);
	virtual ~ErrorInvalidIP() throw();
	virtual const char* what() const throw();
};
