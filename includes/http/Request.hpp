// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<Request.hpp>> -- <<Aida, Ilmari, Milica>>

#pragma once

#include <map>
#include <string>
#include <cstring>
#include <sstream>
#include <exception>

#include "defs.hpp"

class Client;

#define REQUEST_MAX_REQUESTLINE_SIZE	4096
#define REQUEST_MAX_HEADER_SIZE			16384

typedef std::map<std::string, std::string>	headerList;

class Request
{
	private:
		headerList	_headers;

		std::string	_contentType;
		std::string	_remainder;
		std::string	_version;
		std::string	_method;
		std::string	_body;
		std::string	_uri;

		size_t	_contentLength;
		size_t	_maxBodySize;
		size_t	_chunkSize;

		enum {
			REQUESTLINE,
			HEADERS,
			BODY
		}		_parsingStage;

		bool	_trailers;
		bool	_chunked;
		bool	_parsed;
		bool	_valid;

		i32		_errorCode;

		// private methods
		void	_parseRequestLine(std::string line);
		void	_parseHeaders(std::stringstream rawHeaders);
		void	_sanitizeURI(void);

		bool	_processBody(const std::string &rawBody);
		bool	_processChunkedBody(std::stringstream bodySection);

	public:
		Request(void);
		~Request(void);
		Request(const Request &other);
		Request& operator=(const Request &other);
		bool operator==(const Request& other) const;

		// public methods
		void	append(const Client &client, const std::string &reqData);
		void	reset(void);

		// public getters
		const headerList	&getHeaderList(void) const;

		const std::string	&getHeader(const std::string &key) const;
		const std::string	&getContentType(void) const;
		const std::string	&getVersion(void) const;
		const std::string	&getMethod(void) const;
		const std::string	&getBody(void) const;
		const std::string	&getURI(void) const;

		const size_t	&getContentLength(void) const;

		const bool	&isChunked(void) const;
		const bool	&isParsed(void) const;
		const bool	&isValid(void) const;

		const i32	&getErrorCode(void) const;

		class InvalidRequestLineException: public std::exception {
			public:
				const char	*what(void) const noexcept;
		};

		class FieldNotFoundException: public std::exception {
			public:
				const char	*what(void) const noexcept;
		};

		class InvalidHeaderException: public std::exception {
			public:
				const char	*what(void) const noexcept;
		};
};
