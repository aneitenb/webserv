/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aneitenb <aneitenb@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/24 16:55:06 by aneitenb          #+#    #+#             */
/*   Updated: 2025/02/24 17:56:12 by aneitenb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Request.hpp"

Request::Request() : _parsingComplete(false), _isValid(false) {}

Request::~Request() {}

int Request::parseRequest(const std::string& rawRequest)
{
	size_t		headerEnd;
	std::string	headerSection;
	std::string	bodySection;
	size_t		requestLineEnd;
	std::string	requestLine;
	std::string	headerBlock;
	
	headerEnd = rawRequest.find("\r\n\r\n"); //standard HTTP delimiter that separates the headers from the body
	if (headerEnd == std::string::npos)
	{
		//not complete yet
		return 0;
	}
	
	//split requests into header and body
	headerSection = rawRequest.substr(0, headerEnd);
	bodySection = "";
	if (headerEnd + 4 < rawRequest.length())	//checks if there's content after header
		bodySection = rawRequest.substr(headerEnd + 4);	//starts substr after the break
		
	//find end of request line
	requestLineEnd = headerSection.find("\r\n");
	if (requestLineEnd == std::string::npos)
		return -1; //malformed request

	//parse request line for method, URI, HTTP version
	requestLine = headerSection.substr(0, requestLineEnd);
	if (parseRequestLine(requestLine) != 0)
		return -1;

	//parse headers
	headerBlock = headerSection.substr(requestLineEnd + 2);
	if (parseHeaders(headerBlock) != 0)
		return -1;
		
	//parse body if it exists
	if (!bodySection.empty())
	{
		if (parseBody(bodySection) != 0)
			return -1;
	}
	
	_parsingComplete = true;
	_isValid = true;
	return 1;
}

int Request::parseRequestLine(const std::string& line)
{
	return 0;
}
int Request::parseHeaders(const std::string& headerBlock)
{
	return 0;
}

int Request::parseBody(const std::string& bodyContent)
{
	return 0;
}

std::string Request::getMethod() const
{
	
}

std::string Request::getURI() const
{
	
}

std::string Request::getVersion() const
{
	
}

std::string Request::getHeader(const std::string& key) const
{
	
}

std::string Request::getBody() const
{
	
}

bool Request::isComplete() const
{
	
}

bool Request::isValid() const
{
	
}
