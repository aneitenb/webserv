/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aneitenb <aneitenb@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/24 16:54:53 by aneitenb          #+#    #+#             */
/*   Updated: 2025/02/24 16:56:37 by aneitenb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once REQUEST_HPP

#include "Webserv.hpp"

class Request {
private:
	std::string _method;
	std::string _uri;
	std::string _httpVersion;
	std::map<std::string, std::string> _headers;
	std::string _body;
	bool _parsingComplete;
	bool _isValid;

public:
	Request();
	~Request();
	
	// Parsing methods
	int parseRequest(const std::string& rawRequest);
	int parseRequestLine(const std::string& line);
	int parseHeaders(const std::string& headerBlock);
	int parseBody(const std::string& bodyContent);
	
	// Getters
	std::string getMethod() const;
	std::string getURI() const;
	std::string getVersion() const;
	std::string getHeader(const std::string& key) const;
	std::string getBody() const;
	bool isComplete() const;
	bool isValid() const;
};
