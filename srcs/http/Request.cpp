// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<Request.cpp>> -- <<Aida, Ilmari, Milica>>

#include <iostream>
#include "http/Request.hpp"

#define _ERR_BAD_REQUEST		400
#define _ERR_ENTITY_TOO_LARGE	413

#define _find(c, x)	(std::find(c.cbegin(), c.cend(), x))
#define _trimLWS(s)	(s.erase(0, s.find_first_not_of(LWS)), s.erase(s.find_last_not_of(LWS) + 1))

[[maybe_unused]] static inline std::string	_printRawRequest(const std::string &reqData);
static inline bool							_getChunkSize(std::stringstream &bodySection, std::string &remainder, size_t &chunkSize);

Request::Request(const ServerBlock &cfg): _contentLength(0), _chunkSize(0), _parsingStage(REQUESTLINE), _trailers(false), _chunked(false), _parsed(false), _valid(false) {
	this->_maxBodySize = (cfg.hasClientMaxBodySize()) ? cfg.getClientMaxBodySize() : MAX_BODY_SIZE;

}

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
			&& this->_contentLength == other._contentLength && this->_chunkSize == other._chunkSize
			&& this->_parsingStage == other._parsingStage && this->_trailers == other._trailers
			&& this->_chunked == other._chunked && this->_parsed == other._parsed && this->_valid == other._valid
			&& this->_errorCode == other._errorCode) ? true : false;
}

Request::~Request(void) {}

// public methods
void	Request::append(const std::string &reqData) {
	size_t	end;
	bool	fell;

	fell = false;
#ifdef __DEBUG
	std::cerr << SGR_REQUEST << "append: current remainder: {\n" << _printRawRequest(this->_remainder) << "\n}" << SGR_RESET << "\n";
	std::cerr << SGR_REQUEST << "append: new request data: {\n" << _printRawRequest(reqData) << "\n}" << SGR_RESET << "\n";
#endif /* __DEBUG */
	switch (this->_parsingStage) {
		case REQUESTLINE:
			this->_parsed = false;
			this->_remainder += reqData;
			end = this->_remainder.find(CRLF);
			if (end == std::string::npos)
				break ;
			end += 2;
			try {
				this->_valid = true;
				this->_parseRequestLine(this->_remainder.substr(0, end));
			} catch (Request::InvalidRequestLineException &) { this->_valid = false; this->_errorCode = _ERR_BAD_REQUEST; }
			this->_remainder.erase(0, end);
			this->_headers.clear();
			this->_parsingStage = HEADERS;
			fell = true;
			[[fallthrough]];
		case HEADERS:
			if (!fell)
				this->_remainder += reqData;
			end = this->_remainder.find(CRLF CRLF);
			if (end == std::string::npos)
				break ;
			end += 4;
			try {
				this->_parseHeaders(std::stringstream(this->_remainder.substr(0, end)));
			} catch (Request::InvalidHeaderException &) { this->_valid = false; this->_errorCode = _ERR_BAD_REQUEST; }
			this->_remainder.erase(0, end);
			this->_body.clear();
			this->_parsingStage = BODY;
			fell = true;
			[[fallthrough]];
		case BODY:
			if (!fell)
				this->_remainder += reqData;
			if (!this->_processBody(this->_remainder))
				break ;
			this->_parsingStage = REQUESTLINE;
			this->_parsed = true;
	}
}

void	Request::reset(void) {
	this->_parsingStage = REQUESTLINE;
	this->_remainder.clear();
}

// private methods
void	Request::_parseRequestLine(std::string line) {
	static std::regex	validReqLine("(GET|POST|DELETE) +[^\\x00-\\x1F\"#<>{}|\\\\^[\\]`\\x7F]+ +HTTP\\/1\\.1\\r\\n");

#ifdef __DEBUG
	std::cerr << SGR_REQUEST << "_parseRequestLine: Request-Line: " << line.substr(0, line.length() - 2) << SGR_RESET << "\n";
#endif /* __DEBUG */
	if (!std::regex_match(line, validReqLine))
		throw Request::InvalidRequestLineException();
	std::stringstream(line) >> this->_method >> this->_uri >> this->_version;
	this->_decodeURI();
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
#ifdef __DEBUG
			std::cerr << SGR_REQUEST << "_parseHeaders: current header line: " << line << SGR_RESET << "\n";
#endif /* __DEBUG */
			if (line[line.length() - 1] == *CR)
				line.erase(line.length() - 1);
			sep = line.find(":");
			if (sep == std::string::npos)
				valid = false;
			key = line.substr(0, sep);
			val = line.substr(sep + 1);
			_trimLWS(key);
			_trimLWS(val);
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

void	Request::_decodeURI(void) {
	std::string	hexStr;
	std::string	_uri;

	for (auto i = this->_uri.cbegin(); i != this->_uri.cend(); i++) {
		if (*i == '%' && this->_uri.cend() - i > 2) {
			hexStr = this->_uri.substr(i - this->_uri.cbegin() + 1, 2);
			_uri += static_cast<char>(std::stoi(hexStr, 0, 16));
			i += 2;
		} else
			_uri += (*i == '+') ? ' ' : *i;
	}
	this->_uri = _uri;
}

bool	Request::_processBody(const std::string &rawBody) {
	bool	rv;

	if (this->_contentLength == 0 && !this->_chunked) {
		this->_remainder.clear();
		return true;
	}
	if (this->_chunked)
		rv = this->_processChunkedBody(std::stringstream(rawBody));
	else {
#ifdef __DEBUG
		std::cerr << SGR_REQUEST << "_processBody: current body segment: '" << rawBody << "'" << SGR_RESET << "\n";
#endif /* __DEBUG */
		this->_body += rawBody;
		if (this->_body.size() > this->_maxBodySize) {
			this->_valid = false;
			this->_errorCode = _ERR_ENTITY_TOO_LARGE;
			return 1;
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
#ifdef __DEBUG
			std::cerr << SGR_REQUEST << "_processChunkedBody: next chunk size: " << this->_chunkSize << SGR_RESET << "\n";
#endif /* __DEBUG */
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
#ifdef __DEBUG
				std::cerr << SGR_REQUEST << "_processChunkedBody: next chunk data: '" << chunkData << "'" << SGR_RESET << "\n";
#endif /* __DEBUG */
				this->_contentLength += this->_chunkSize;
				this->_body += chunkData;
				parsingStage = CHUNKSIZE;
				if (this->_body.size() > this->_maxBodySize) {
					this->_valid = false;
					this->_errorCode = _ERR_ENTITY_TOO_LARGE;
					return true;
				}
				if (!_getChunkSize(bodySection, this->_remainder, this->_chunkSize))
					return false;
#ifdef __DEBUG
				std::cerr << SGR_REQUEST << "_processChunkedBody: next chunk size: " << this->_chunkSize << SGR_RESET << "\n";
#endif /* __DEBUG */
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
			} catch (Request::InvalidHeaderException &) { this->_valid = false; this->_errorCode = _ERR_BAD_REQUEST; }
			parsingStage = CHUNKSIZE;
	}
	this->_chunked = false;
	return true;
}

[[maybe_unused]] static inline std::string	_printRawRequest(const std::string &reqData) {
	std::string	escaped;

	escaped.clear();;
	for (const char c : reqData) {
		if (isprint(c) || c == '\n')
			escaped += c;
		else if (c == '\r')
			escaped += "\\r";
		else if (c && c < ' ') {
			escaped += '^';
			escaped += c + '@';
		}
	}
	return escaped;
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

const bool	&Request::isValid(void) const { return this->_valid; }

const i32	&Request::getErrorCode(void) const { return this->_errorCode; }

//exceptions
const char	*Request::InvalidRequestLineException::what(void) const noexcept { return "Invalid request line"; }

const char	*Request::FieldNotFoundException::what(void) const noexcept { return "Header field not found"; }

const char	*Request::InvalidHeaderException::what(void) const noexcept { return "Invalid header"; }
