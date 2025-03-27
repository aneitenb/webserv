/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ivalimak <ivalimak@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 17:06:20 by ivalimak          #+#    #+#             */
/*   Updated: 2025/03/27 15:33:24 by ivalimak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <array>
#include "http/Request.hpp"

#define _find(c, x)	(std::find(c.cbegin(), c.cend(), x))
#define _trimLWS(s)	(s.erase(0, s.find_first_not_of("\r\n \t")), s.erase(s.find_last_not_of("\r\n \t") + 1))

Request::Request(const std::string &rawRequest): _contentLength(0), _chunked(false) {
	std::string	bodySection;
	size_t		requestEnd;
	size_t		headerEnd;

	requestEnd = rawRequest.find(CRLF);
	if (requestEnd == std::string::npos)
		throw Request::InvalidRequestLineException();
	headerEnd = rawRequest.find(CRLF CRLF, requestEnd + 2);
	if (headerEnd == std::string::npos)
		throw Request::IncompleteHeaderException();
	if (!this->_parseRequestLine(std::stringstream(rawRequest.substr(0, requestEnd))))
		throw Request::InvalidRequestLineException();
	if (!this->_parseHeaders(std::stringstream(rawRequest.substr(requestEnd + 2))))
		throw Request::InvalidHeaderException();
	try {
		this->_chunked = this->getHeader("Transfer-Encoding") == "chunked";
	} catch (std::exception) {}
	try {
		std::string _clstr = this->getHeader("Content-Length");
		this->_contentLength = std::stoul(_clstr);
	} catch (std::exception) {} // separate catch for possible exception from stoul?
	try {
		this->_contentType = this->getHeader("Content-Type");
	} catch (std::exception) {} // is missing Content-Type an error ?
	bodySection = (rawRequest.length() > headerEnd + 4) ? rawRequest.substr(headerEnd + 4) : "";
	if (!bodySection.empty()) {
		bool _rv = (this->_chunked) ? this->_processChunkedBody(std::stringstream(bodySection)) : this->_parseBody(bodySection);
		if (!_rv)
			throw Request::InvalidBodyException();
	}
	this->_parsed = true;
}

Request::~Request(void) {}

// private methods
bool Request::_processChunkedBody(std::stringstream bodySection) {
	std::string	chunkSizeStr;
	std::string chunkData;
	size_t		chunkSize;
	size_t		length;
	char		*Data;

	length = 0;
	std::getline(bodySection, chunkSizeStr);
	if (chunkSizeStr.empty() || chunkSizeStr[chunkSizeStr.length() - 1] != *CR)
		return false;
	chunkSize = std::stoul(chunkSizeStr.substr(0, chunkSizeStr.find(";\t")), 0, 16);
	while (chunkSize > 0) {
		Data = new char[chunkSize + 3];
		bodySection.read(Data, chunkSize + 2);
		chunkData = std::string(Data);
		if (chunkData.length() != chunkSize + 2 || chunkData.substr(chunkSize, 2) != CRLF)
			return false;
		this->_body += std::string(Data).substr(0, chunkSize);
		length += chunkSize;
		delete[] Data;
		std::getline(bodySection, chunkSizeStr);
		if (chunkSizeStr.empty() || chunkSizeStr[chunkSizeStr.length() - 1] != *CR)
			return false;
		chunkSize = std::stoul(chunkSizeStr.substr(0, chunkSizeStr.find(";\t")), 0, 16);
	}
	// entity header ?
	this->_contentLength = length;
	this->_chunked = 0;
	return true;
}

std::string	Request::_decodeURI(const std::string &uri) {
	std::string	hexStr;
	std::string	_uri;

	for (auto i = uri.cbegin(); i != uri.cend(); i++) {
		if (*i == '%' && uri.cend() - i > 2) {
			hexStr = uri.substr(i - uri.cbegin() + 1, 2);
			_uri += static_cast<char>(std::stoi(hexStr, 0, 16));
			i += 2;
		} else
			_uri += (*i == '+') ? ' ' : *i;
	}
	return _uri;
}

bool	Request::_parseRequestLine(std::stringstream line) {
	static const std::array<std::string, 3>	_validMethods = {"GET", "POST", "DELETE"};

	if (!(line >> this->_method >> this->_uri >> this->_version))
		return false;
	return (_find(_validMethods, this->_method) && this->_version == "HTTP/1.1") ? true : false;
}

bool	Request::_parseHeaders(std::stringstream rawHeaders) {
	std::string	line;
	std::string	key;
	std::string	val;
	size_t		sep;

	while (std::getline(rawHeaders, line)) {
		if (!line.empty() && line != "\r") {
			if (line[line.length() - 1] == *CR)
				line.erase(line.length() - 1);
			sep = line.find(":");
			if (sep == std::string::npos)
				return false;
			key = line.substr(0, sep);
			val = line.substr(sep + 1);
			_trimLWS(key);
			_trimLWS(val);
			this->_headers[key] = val;
		}
	}
	return true;
}

bool	Request::_parseBody(const std::string &rawBody) {
	this->_body = rawBody;
	if (this->_contentLength) {
		if (this->_body.size() < this->_contentLength)
			return false;
		this->_body = this->_body.substr(0, this->_contentLength);
	}
	return true;
}

// public getters
const headerlist_t	&Request::getHeaderList(void) const { return this->_headers; }

const std::string	&Request::getHeader(const std::string &key) const {
	auto	field = this->_headers.find(key);

	if (field == this->_headers.end())
		throw Request::FieldNotFoundException();
	return field->second;
}

const std::string	&Request::getResourcePath(void) const { return this->_resourcePath; }

const std::string	&Request::getContentType(void) const { return this->_contentType; }

const std::string	&Request::getVersion(void) const { return this->_version; }

const std::string	&Request::getMethod(void) const { return this->_method; }

const std::string	&Request::getBody(void) const { return this->_body; }

const std::string	&Request::getURI(void) const { return this->_uri; }

const size_t	&Request::getContentLength(void) const { return this->_contentLength; }

const bool	&Request::isChunked(void) const { return this->_chunked; }

const bool	&Request::isParsed(void) const { return this->_parsed; }

//exceptions
const char	*Request::InvalidRequestLineException::what(void) const noexcept { return "Invalid or missing request line"; }

const char	*Request::IncompleteHeaderException::what(void) const noexcept { return "Incomplete or missing header"; }

const char	*Request::FieldNotFoundException::what(void) const noexcept { return "Header field not found"; }

const char	*Request::InvalidHeaderException::what(void) const noexcept { return "Invalid header"; }

const char	*Request::InvalidBodyException::what(void) const noexcept { return "Invalid or incomplete body"; }
