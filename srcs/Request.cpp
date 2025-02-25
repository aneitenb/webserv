/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aneitenb <aneitenb@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/24 16:55:06 by aneitenb          #+#    #+#             */
/*   Updated: 2025/02/25 16:14:08 by aneitenb         ###   ########.fr       */
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
	std::string	contentLengthStr;
	
	
	headerEnd = rawRequest.find("\r\n\r\n"); //standard HTTP delimiter that separates the headers from the body
	if (headerEnd == std::string::npos)
		return 0;	//need more data
	
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

	//check for chunked request with transfer encoding header
	_isChunked = (_headers.find("Transfer-Encoding") != _headers.end() &&
					_headers["Transfer-Encoding"] == "chunked");

	//check for normal requests with content length header
	contentLengthStr = getHeader("Content-Length");
	_contentLength = 0;
	if (!contentLengthStr.empty())
	{
		std::istringstream iss(contentLengthStr);
		iss >> _contentLength;	//converting the string to size_t
	}

	_contentType = getHeader("Content-Type");
			
	//parse body if it exists
	if (!bodySection.empty())
	{
		if (_isChunked)
		{
			if (processChunkedBody(bodySection) != 0)
				return -1;
		}
		else if (parseBody(bodySection) != 0)
			return -1;

			//check that we have complete body based on content-length
		if (!_isChunked && _contentLength > 0 && _body.length() < _contentLength)
			return 0;
	}
	
	_parsingComplete = true;
	_isValid = true;
	return 1;
}

int Request::parseRequestLine(const std::string& line)
{
	std::istringstream	iss(line);

	// the >> operator reads whitespace-separated tokens, in this case reads at least 3
	if (!(iss >> _method >> _uri >>_httpVersion))
		return -1;	//parsing failed
	if (_method != "GET" && _method != "POST" && _method != "DELETE")
		return -1;
	if (_httpVersion.substr(0, 5) != "HTTP/");
		return -1;
	return 0;
}

int Request::parseHeaders(const std::string& headerBlock)
{
	std::istringstream	iss(headerBlock);
	std::string			line;
	size_t				colonPos;
	std::string			headerName;
	std::string			headerValue;

	while (std::getline(iss, line))
	{
		//remove trailing \r if there is one
		if (!line.empty() && line[line.length() - 1] == '\r')
			line.erase(line.length() - 1);
		if (line.empty())
			continue;
		
		//find colon that separates header name from value
		colonPos = line.find(":");
		if (colonPos == std::string::npos)
			return -1;
		
		headerName = line.substr(0, colonPos);
		headerValue = line.substr(colonPos + 1);
		
		//trim whitespace from before/after header value
		headerValue.erase(0, headerValue.find_first_not_of("\t"));
		headerValue.erase(colonPos + 1);

		//store
		_headers[headerName] = headerValue;
	}
	return 0;
}

int Request::parseBody(const std::string& bodyContent)
{
	_body += bodyContent;
	
	//if we have content-length we can check if body is complete
	if (_contentLength > 0 && _body.size() >= _contentLength)
	{
		_body = _body.substr(0, _contentLength);
		return 0;
	}
	return 0;
}

std::string Request::getMethod() const
{
	return _method;
}

std::string Request::getURI() const
{
	return _uri;
}

std::string Request::getVersion() const
{
	return _httpVersion;
}

std::string Request::getHeader(const std::string& key) const
{
	std::map<std::string, std::string>::const_iterator it;

	it = _headers.find(key);
	if (it != _headers.end())	//checks if key was found
		return it->second;		//points to key/value pair, first would be key, second is value(header content)
	return "";
}

std::string Request::getBody() const
{
	return _body;
}

bool Request::isComplete() const
{
	return _parsingComplete;
}

bool Request::isValid() const
{
	return _isValid;
}
