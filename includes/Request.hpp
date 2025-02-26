/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aneitenb <aneitenb@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/24 16:54:53 by aneitenb          #+#    #+#             */
/*   Updated: 2025/02/26 15:50:04 by aneitenb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once REQUEST_HPP

#include "Webserv.hpp"

class Request {
private:
	std::map<std::string, std::string> _headers;
	std::string _method;
	std::string _uri;
	std::string _httpVersion;
	std::string _body;
	std::string	_contentType;
	std::string	_boundry;
	std::string	_partialBody;
	std::string	_resourcePath;
	size_t		_contentLength;
	bool 		_parsingComplete;
	bool 		_isValid;
	bool 		_isChunked;

public:
	Request();
	~Request();
	
	// Parsing methods
	int parseRequest(const std::string& rawRequest);
	int parseRequestLine(const std::string& line);
	int parseHeaders(const std::string& headerBlock);
	int parseBody(const std::string& bodyContent);
	int	processChunkedBody(std::string& bodyContent);
	int	processGetRequest();
	std::string decodeURI(const std::string& uri);
	
	std::string	getMethod() const;
	std::string getURI() const;
	std::string getVersion() const;
	std::string getHeader(const std::string& key) const;
	std::string getBody() const;
	std::string getContentType() const;
	std::string	getResourcePath() const;
	size_t		getContentLength() const;
	
	bool	isComplete() const;
	bool	isValid() const;
	bool	isChunked() const;
};
