// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<Request.cpp>> -- <<Aida, Ilmari, Milica>>

#include "http/Request.hpp"

#define _find(c, x)	(std::find(c.cbegin(), c.cend(), x))
#define _trimLWS(s)	(s.erase(0, s.find_first_not_of(LWS)), s.erase(s.find_last_not_of(LWS) + 1))

static inline bool	_getChunkSize(std::stringstream &bodySection, std::string &remainder, size_t &chunkSize);

Request::Request(void): _contentLength(0), _parsingStage(REQUESTLINE), _chunked(false) {}

Request::~Request(void) {}

// public methods
void	Request::append(const std::string &reqData) {
	size_t				end;

	end = 0;
	switch (this->_parsingStage) {
		case REQUESTLINE:
			end = reqData.find(CRLF);
			if (end == std::string::npos) {
				this->_remainder += reqData; // check if this->remainder now contains CRLF
				break ;
			}
			try {
				this->_parseRequestLine(this->_remainder + reqData.substr(0, end));
				this->_remainder.clear();
			} catch (Request::InvalidRequestLineException &) {} // store error code (and return ?)
			this->_headers.clear();
			this->_parsingStage = HEADERS;
			[[fallthrough]];
		case HEADERS:
			end = reqData.find(CRLF CRLF, (end) ? end + 2 : 0);
			if (end == std::string::npos) {
				this->_remainder += reqData; // check if this->remainder now contains CRLF CRLF
				break ;
			}
			try {
				this->_parseHeaders(std::stringstream(reqData.substr(0, end)));
				this->_remainder.clear();
			} catch (Request::InvalidHeaderException &) {} // store error code (and return ?)
			this->_body.clear();
			this->_parsingStage = BODY;
			[[fallthrough]];
		case BODY:
			if (!this->_processBody(reqData))
				break ;
			this->_parsingStage = REQUESTLINE;
			this->_parsed = true;
	}
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

void	Request::_parseRequestLine(std::string line) {
	static std::regex	validReqLine("(GET|POST|DELETE) +[^\\x00-\\x1F\"#<>{}|\\\\^[\\]`\\x7F]+ +HTTP\\/1\\.1\r\n");

	if (!std::regex_match(line, validReqLine))
		throw Request::InvalidRequestLineException();
	std::stringstream(line) >> this->_method >> this->_uri >> this->_version;
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
	catch (std::exception &) {} // set error code
	if (!valid)
		throw Request::InvalidHeaderException();
}

bool	Request::_processBody(const std::string &rawBody) {
	bool	rv;

	if (this->_contentLength == 0 && !this->_chunked)
		return true;
	if (this->_chunked)
		rv = this->_processChunkedBody(std::stringstream(rawBody));
	else {
		this->_body += rawBody;
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
	static size_t	chunkSize = 0;
	std::string		chunkData;

	switch (parsingStage) {
		case CHUNKSIZE:
			if (!_getChunkSize(bodySection, this->_remainder, chunkSize))
				return false;
			this->_remainder.clear();
			parsingStage = CHUNK;
			[[fallthrough]];
		case CHUNK:
			while (chunkSize) {
				chunkData.resize(chunkSize);
				bodySection.read(&chunkData[0], chunkSize);
				if (chunkData.length() != chunkSize)
					return false;
				this->_contentLength += chunkSize;
				this->_body += chunkData;
				parsingStage = CHUNKSIZE;
				if (!_getChunkSize(bodySection, this->_remainder, chunkSize))
					return false;
				parsingStage = CHUNK;
			}
			this->_remainder.clear();
			parsingStage = (this->_trailers) ? TRAILERS : CHUNKSIZE;
			if (parsingStage == CHUNKSIZE)
				break ;
			[[fallthrough]];
		case TRAILERS: // TODO trailer handling
			while (!bodySection.eof()) {
				std::getline(bodySection, chunkData);
				this->_remainder += chunkData;
			}
			if (this->_remainder.find(CRLF CRLF) == std::string::npos)
				return false;
			try {
				this->_parseHeaders(std::stringstream(this->_remainder));
				this->_remainder.clear();
			} catch (Request::InvalidHeaderException &) {} // store error code
			parsingStage = CHUNKSIZE;
	}
	this->_chunked = false;
	return true;
}

static inline bool	_getChunkSize(std::stringstream &bodySection, std::string &remainder, size_t &chunkSize) {
	std::string	chunkSizeStr;

	std::getline(bodySection, chunkSizeStr);
	if (*chunkSizeStr.end() != *CR) {
		remainder += chunkSizeStr;
		return false;
	}
	remainder.clear();
	try { chunkSize = std::stoul(chunkSizeStr.substr(0, chunkSizeStr.find(";" CR)), 0, 16); }
	catch (std::exception &) { return false; } // store error (invalid chunk size line)
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

const char	*Request::InvalidFieldException::what(void) const noexcept { return "Invalid header field"; }

const char	*Request::InvalidBodyException::what(void) const noexcept { return "Invalid or incomplete body"; }
