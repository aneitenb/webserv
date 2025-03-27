/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ivalimak <ivalimak@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/20 16:46:50 by ivalimak          #+#    #+#             */
/*   Updated: 2025/03/27 15:17:38 by ivalimak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "defs.hpp"

#include <map>
#include <string>
#include <sstream>
#include <algorithm>
#include <exception>

#define CR	"\r"
#define LF	"\n"
#define SP	" "
#define HT	"\t"

#define CRLF CR LF
#define LWS CRLF SP HT

typedef std::map<std::string, std::string>	headerlist_t;

class Request
{
	private:
		headerlist_t	_headers;

		std::string	_resourcePath;
		std::string	_partialBody;
		std::string	_contentType;
		std::string	_version;
		std::string	_method;
		std::string	_body;
		std::string	_uri;

		size_t	_contentLength;

		bool	_chunked;
		bool	_parsed;

		std::string	_decodeURI(const std::string &uri);

		bool	_processChunkedBody(std::stringstream bodySection);
		bool	_parseRequestLine(std::stringstream line);
		bool	_parseHeaders(std::stringstream rawHeaders);
		bool	_parseBody(const std::string &rawBody);

	public:
		Request(void) = delete;
		Request(const std::string &rawRequest);
		~Request(void);

		// public getters
		const headerlist_t	&getHeaderList(void) const;

		const std::string	&getHeader(const std::string &key) const;
		const std::string	&getResourcePath(void) const;
		const std::string	&getContentType(void) const;
		const std::string	&getVersion(void) const;
		const std::string	&getMethod(void) const;
		const std::string	&getBody(void) const;
		const std::string	&getURI(void) const;

		const size_t	&getContentLength(void) const;

		const bool	&isChunked(void) const;
		const bool	&isParsed(void) const;

		class InvalidRequestLineException: public std::exception {
			public:
				const char	*what(void) const noexcept;
		};

		class IncompleteHeaderException: public std::exception {
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

		class InvalidBodyException: public std::exception {
			public:
				const char	*what(void) const noexcept;
		};
};
