// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<Request.cpp>> -- <<Aida, Ilmari, Milica>>

#include <regex>

#include "http/Request.hpp"
#include "server/Client.hpp"
#include "utils/message.hpp"

#undef Info
#define Info(msg)	(info(std::stringstream("") << msg, COLOR_REQUEST))

#define _find(c, x)	(std::find(c.cbegin(), c.cend(), x))
#define _trimLWS(s)	(s.erase(0, s.find_first_not_of(LWS)), s.erase(s.find_last_not_of(LWS) + 1))

#define setError(err)	(this->_errorCode = err, this->_parsed = true, this->_valid = false)

static inline bool	_getChunkSize(std::stringstream &bodySection, std::string &remainder, size_t &chunkSize);

Request::Request(void): _contentLength(0), _maxBodySize(0), _chunkSize(0), _parsingStage(REQUESTLINE), _trailers(false), _chunked(false), _parsed(false), _valid(false), _errorCode(0) {}

Request::Request(const Request &other) {
	*this = other;
}

Request& Request::operator=(const Request &other) {
	if (this != &other) {
		this->_headers = other._headers;
		this->_contentType = other._contentType;
		this->_remainder = other._remainder;
		this->_version = other._version;
		this->_method = other._method;
		this->_body = other._body;
		this->_uri = other._uri;
		this->_contentLength = other._contentLength;
		this->_maxBodySize = other._maxBodySize;
		this->_chunkSize = other._chunkSize;
		this->_parsingStage = other._parsingStage;
		this->_trailers = other._trailers;
		this->_chunked = other._chunked;
		this->_parsed = other._parsed;
		this->_valid = other._valid;
		this->_errorCode = other._errorCode;
	}
	return *this;
}

bool Request::operator==(const Request &other) const {
	return (this->_headers == other._headers && this->_contentType == other._contentType
			&& this->_remainder == other._remainder && this->_version == other._version
			&& this->_method == other._method && this->_body == other._body && this->_uri == other._uri
			&& this->_contentLength == other._contentLength && this->_maxBodySize == other._maxBodySize
			&& this->_chunkSize == other._chunkSize && this->_parsingStage == other._parsingStage
			&& this->_trailers == other._trailers && this->_chunked == other._chunked
			&& this->_parsed == other._parsed && this->_valid == other._valid
			&& this->_errorCode == other._errorCode) ? true : false;
}

Request::~Request(void) {}

// public methods
void	Request::append(const Client &client, const std::string &reqData) {
	const ServerBlock	*serverConf;
	size_t				end;

	this->_remainder += reqData;
	switch (this->_parsingStage) {
		case REQUESTLINE:
			this->_parsed = false;
			end = this->_remainder.find(CRLF);
			if (end == std::string::npos) {
				if (this->_remainder.size() > REQUEST_MAX_REQUESTLINE_SIZE)
					setError(HTTP_URI_TOO_LONG);
				break ;
			}
			end += 2;
			if (end > REQUEST_MAX_REQUESTLINE_SIZE) {
				setError(HTTP_URI_TOO_LONG);
				break ;
			}
			try {
				this->_valid = true;
				this->_parseRequestLine(this->_remainder.substr(0, end));
			} catch (Request::InvalidRequestLineException &) { this->_parsed = true; this->_valid = false; this->_errorCode = HTTP_BAD_REQUEST; }
			this->_remainder.erase(0, end);
			this->_headers.clear();
			this->_parsingStage = HEADERS;
			[[fallthrough]];
		case HEADERS:
			end = this->_remainder.find(CRLF CRLF);
			if (end == std::string::npos) {
				if (this->_remainder.size() > REQUEST_MAX_HEADER_SIZE)
					setError(HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE);
				break ;
			}
			end += 4;
			if (end > REQUEST_MAX_HEADER_SIZE) {
				setError(HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE);
				break ;
			}
			try {
				this->_parseHeaders(std::stringstream(this->_remainder.substr(0, end)));
			} catch (Request::InvalidHeaderException &) { this->_parsed = true; this->_valid = false; this->_errorCode = HTTP_BAD_REQUEST; }
			this->_remainder.erase(0, end);
			this->_body.clear();
			this->_parsingStage = BODY;
			serverConf = client.getSBforResponse(client.getHost());
			this->_maxBodySize = (serverConf) ? serverConf->getClientMaxBodySize() : MAX_BODY_SIZE;
			[[fallthrough]];
		case BODY:
			if (!this->_processBody(this->_remainder))
				return ;
			this->_parsingStage = REQUESTLINE;
			this->_parsed = true;
	}
	if (this->_parsed) {
		if (!this->_valid) {
			Info("\nInvalid request received: Error " << this->_errorCode);
			return ;
		}
		info("\nNew request received:", COLOR_REQUEST);
		Info("  Method:  " << this->_method);
		Info("  URI:     " << this->_uri);
		Info("  Version: " << this->_version);
#ifdef __DEBUG_REQ_SHOW_HEADERS
		info("\n  Headers:", COLOR_REQUEST);
		for (const auto &field : this->_headers)
			Info("    " << field.first << ": " << field.second);
#endif /* __DEBUG_REQ_SHOW_HEADERS */
#ifdef __DEBUG_REQ_SHOW_BODY
		if (this->_body.size() != 0) {
			info("\n  Body:", COLOR_REQUEST);
			printBody(this->_contentType, this->_body, COLOR_REQUEST);
		}
#endif /* __DEBUG_REQ_SHOW_BODY */
	}
}


void	Request::reset(void) {
	this->_parsingStage = REQUESTLINE;
	this->_remainder.clear();
}

// private methods
void	Request::_parseRequestLine(std::string line) {
	static std::regex	validReqLine("([A-Z]+) +[^\\x00-\\x1F\"#<>{}|\\\\^[\\]`\\x7F]+ +HTTP\\/1\\.1\\r\\n");

	if (!std::regex_match(line, validReqLine))
		throw Request::InvalidRequestLineException();
	std::stringstream(line) >> this->_method >> this->_uri >> this->_version;
	this->_sanitizeURI();
}

void	Request::_parseHeaders(std::stringstream rawHeaders) {
	std::string	line;
	std::string	key;
	std::string	val;
	size_t		sep;
	bool		valid;

	valid = true;
	while (std::getline(rawHeaders, line)) {
		if (!line.empty() && line != CR) {
			if (line[line.length() - 1] == *CR)
				line.erase(line.length() - 1);
			sep = line.find(":");
			if (sep == std::string::npos)
				valid = false;
			key = line.substr(0, sep);
			val = line.substr(sep + 1);
			_trimLWS(key);
			_trimLWS(val);
			if (val.empty()) {
				valid = false;
				continue ;
			}
			if (this->_headers.find(key) == this->_headers.end())
				this->_headers[key] = val;
			else
				this->_headers[key].append(val);
		}
	}
	try { this->_chunked = this->getHeader("Transfer-Encoding") == "chunked"; }
	catch (Request::FieldNotFoundException &) { this->_chunked = false; }
	try { this->_trailers = this->getHeader("Trailers") != ""; }
	catch (Request::FieldNotFoundException &) { this->_trailers = false; }
	try { this->_contentType = this->getHeader("Content-Type"); }
	catch (Request::FieldNotFoundException &) { this->_contentType = "application/octet-stream"; } // 2616/7.2.1
	try { this->_contentLength = std::stoul(this->getHeader("Content-Length")); }
	catch (Request::FieldNotFoundException &) { this->_contentLength = 0; }
	catch (std::exception &) { valid = false; }
	if (!valid)
		throw Request::InvalidHeaderException();
}

void	Request::_sanitizeURI(void) {
	std::string::const_iterator	i;
	std::string					uri;
	std::string					hexStr;
	bool						onDirBoundary;

	for (onDirBoundary = false, i = this->_uri.cbegin(); i != this->_uri.cend(); i++) {
		if (*i == '%' && this->_uri.cend() - i > 2) {
			hexStr = this->_uri.substr(i - this->_uri.cbegin() + 1, 2);
			uri += static_cast<char>(std::stoi(hexStr, 0, 16));
			i += 2;
			onDirBoundary = false;
		} else if (*i == '/') {
			if (!onDirBoundary)
				uri += *i;
			onDirBoundary = true;
		} else {
			uri += (*i == '+') ? ' ' : *i;
			onDirBoundary = false;
		}
	}
	this->_uri = uri;
}

bool	Request::_processBody(const std::string &rawBody) {
	bool	rv;

	if (this->_contentLength == 0 && !this->_chunked) {
		if (rawBody.size() != 0) {
			try {
				this->getHeader("Content-Length");
			} catch (Request::FieldNotFoundException &) {
				this->_errorCode = HTTP_LENGTH_REQUIRED;
				this->_valid = false;
			}
		}
		this->_remainder.clear();
		return true;
	}
	if (this->_chunked)
		rv = this->_processChunkedBody(std::stringstream(rawBody));
	else {
		this->_body += rawBody;
		if (this->_body.size() > this->_maxBodySize) {
			this->_errorCode = HTTP_PAYLOAD_TOO_LARGE;
			this->_valid = false;
			return true;
		}
		this->_remainder.clear();
		rv = this->_body.size() >= this->_contentLength;
		if (rv && this->_body.size() > this->_contentLength) {
			this->_remainder = this->_body.substr(this->_contentLength);
			this->_body.erase(this->_contentLength);
		}
	}
	return rv;
}

bool	Request::_processChunkedBody(std::stringstream bodySection) {
	static enum {
		CHUNKSIZE,
		CHUNK,
		TRAILERS
	}				parsingStage = CHUNKSIZE;
	std::string		chunkData;
	size_t			dataLen;

	switch (parsingStage) {
		case CHUNKSIZE:
			if (!_getChunkSize(bodySection, this->_remainder, this->_chunkSize))
				return false;
			this->_remainder.clear();
			parsingStage = CHUNK;
			[[fallthrough]];
		case CHUNK:
			while (this->_chunkSize) {
				chunkData.resize(this->_chunkSize);
				bodySection.read(&chunkData[0], this->_chunkSize);
				dataLen = std::strlen(chunkData.c_str()); // use strlen because chunkData.length() isn't actually the length of chunkData
				if (dataLen != this->_chunkSize) {
					this->_remainder = chunkData.c_str();
					return false ;
				}
				this->_contentLength += this->_chunkSize;
				this->_body += chunkData;
				parsingStage = CHUNKSIZE;
				if (this->_body.size() > this->_maxBodySize) {
					this->_valid = false;
					this->_errorCode = HTTP_PAYLOAD_TOO_LARGE;
					return true;
				}
				if (!_getChunkSize(bodySection, this->_remainder, this->_chunkSize))
					return false;
				parsingStage = CHUNK;
			}
			this->_remainder.clear();
			parsingStage = (this->_trailers) ? TRAILERS : CHUNKSIZE;
			if (parsingStage == CHUNKSIZE)
				break ;
			[[fallthrough]];
		case TRAILERS:
			while (!bodySection.eof()) {
				std::getline(bodySection, chunkData);
				this->_remainder += chunkData;
			}
			if (this->_remainder.find(CRLF CRLF) == std::string::npos)
				return false;
			try {
				this->_parseHeaders(std::stringstream(this->_remainder));
				this->_remainder.clear();
			} catch (Request::InvalidHeaderException &) { this->_valid = false; this->_errorCode = HTTP_BAD_REQUEST; }
			parsingStage = CHUNKSIZE;
	}
	this->_chunked = false;
	return true;
}

static inline bool	_getChunkSize(std::stringstream &bodySection, std::string &remainder, size_t &chunkSize) {
	std::string	chunkSizeStr;

	std::getline(bodySection, chunkSizeStr);
	if (chunkSizeStr.empty() || chunkSizeStr.at(chunkSizeStr.length() - 1) != *CR) {
		remainder += chunkSizeStr;
		return false;
	}
	remainder.clear();
	try { chunkSize = std::stoul(chunkSizeStr.substr(0, chunkSizeStr.find(";" CR)), 0, 16); }
	catch (std::exception &) { return false; }
	return true;
}

// public setters
void	Request::setErrorCode(const i16 errorCode) { this->_errorCode = errorCode; }

// public getters
const headerList	&Request::getHeaderList(void) const { return this->_headers; }

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

const bool	&Request::isValid(void) const { return this->_valid; }

const i16	&Request::getErrorCode(void) const { return this->_errorCode; }

//exceptions
const char	*Request::InvalidRequestLineException::what(void) const noexcept { return "Invalid request line"; }

const char	*Request::FieldNotFoundException::what(void) const noexcept { return "Header field not found"; }

const char	*Request::InvalidHeaderException::what(void) const noexcept { return "Invalid header"; }
