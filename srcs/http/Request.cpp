/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ivalimak <ivalimak@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 17:06:20 by ivalimak          #+#    #+#             */
/*   Updated: 2025/04/03 16:57:26 by ivalimak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <array>
#include "http/Request.hpp"

#define _find(c, x)	(std::find(c.cbegin(), c.cend(), x))
#define _trimLWS(s)	(s.erase(0, s.find_first_not_of(LWS)), s.erase(s.find_last_not_of(LWS) + 1))

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
		bool _rv = (this->_chunked) ? this->processChunkedBody(std::stringstream(bodySection)) : this->_parseBody(bodySection);
		if (!_rv)
			throw Request::InvalidBodyException();
	}
	this->_parsed = !this->_chunked;
}

Request::~Request(void) {}

// public methods
bool	Request::processChunkedBody(std::stringstream bodySection) {
	std::string	entityHeaders;
	std::string	chunkSizeStr;
	std::string	headerLine;
	std::string	chunkData;
	size_t		chunkSize;

	std::getline(bodySection, chunkSizeStr);
	if (chunkSizeStr.empty() || *chunkSizeStr.end() != *CR)
		return false;
	try {
		chunkSize = std::stoul(chunkSizeStr.substr(0, chunkSizeStr.find(";" CR)), 0, 16);
	} catch (std::exception &) { return false; } // invalid chunk size line
	if (chunkSize) {
		chunkData.resize(chunkSize);
		bodySection.read(&chunkData[0], chunkSize);
		if (chunkData.length() != chunkSize)
			return false;
		this->_contentLength += chunkSize;
		this->_body += chunkData;
	} else {
		entityHeaders = "";
		for (std::getline(bodySection, headerLine); !headerLine.empty();std::getline(bodySection, headerLine))
			entityHeaders += headerLine;
		this->_parseHeaders(std::stringstream(entityHeaders));
		this->_chunked = false;
		this->_parsed = true;
	}
	return true;
}

// private methods
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
	if (!(line >> this->_method >> this->_uri >> this->_version))
		return false;
	return true;
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
