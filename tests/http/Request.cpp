// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<Request.cpp>> -- <<Aida, Ilmari, Milica>>

#include <iomanip>
#include <iostream>
#include "http/Request.hpp"

#define _validstr(b)	((b) ? "\x1b[1;38;5;42mValid" SGR_INFO : "\x1b[1;38;5;88mInvalid" SGR_INFO)

#define SGR_INFO	"\x1b[1;38;5;118m"
#define SGR_RESET	"\x1b[m"

static inline void	_checkValid(const Request &req, const bool &expected, bool &err, const size_t &n);
static inline void	_multiAppend(Request &req);
static inline void	_chunkedPost(Request &req);
static inline void	_printRequest(const Request &req, const size_t &n);
static inline void	_printBody(const Request &req);

i32	main(void) {
	bool	err;
	Request	req;

	err = false;
	std::cerr << SGR_INFO << "---REQUEST 1---" << SGR_RESET << "\n";
	req.append("GET datatracker.ietf.org/doc/html/rfc2616 HTTP/1.1\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif/image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0,7\r\nAccept-Encoding: gzip,deflate,br,zstd\r\nAccept-Language: en-US,en;q=0.9\r\nCache-Control: max-age=0\r\nPriority: u=0,i\r\nSec-Ch-Ua: \"Google Chrome\";v=\"131\",\"Chromium\";v=\"131\",\"Not_A_BRAND\";v=\"24\"\r\nSec-Ch-Ua-Mobile: ?0\r\nSec-Ch-Ua-Platform: \"Linux\"\r\nSec-Fetch-Dest: document\r\nSec-Fetch-Mode: navigate\r\nSec-Fetch-Site: same-origin\r\nSec-Fetch-User: ?1\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.36\");\r\n\r\n");
	_printRequest(req, 1);
	_checkValid(req, true, err, 1);
	std::cerr << SGR_INFO << "---REQUEST 2---" << SGR_RESET << "\n";
	req.append("GET datatracker.ietf.org/doc/html/rfc2616 HTTP/1.1\r\n\r\n\r\n");
	_printRequest(req, 2);
	_checkValid(req, true, err, 2);
	std::cerr << SGR_INFO << "---REQUEST 3---" << SGR_RESET << "\n";
	req.append("GET datatracker.ietf.org/doc/html/rfc2616 HTTP/1.1\r\nContent-Length: seven\r\n\r\n");
	_printRequest(req, 3);
	_checkValid(req, false, err, 3);
	std::cerr << SGR_INFO << "---REQUEST 4---" << SGR_RESET << "\n";
	_multiAppend(req);
	_checkValid(req, true, err, 4);
	std::cerr << SGR_INFO << "---REQUEST 5---" << SGR_RESET << "\n";
	_chunkedPost(req);
	_checkValid(req, true, err, 5);
	std::cerr << SGR_INFO << "---REQUEST 6---" << SGR_RESET << "\n";
	req.append("GET datatracker.ietf.org/doc/html/rfc2616 HTTP/1.1\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif/image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0,7\r\nAccept-Encoding: gzip,deflate,br,zstd\r\nAccept-Language: en-US,en;q=0.9\r\nCache-Control: max-age=0\r\nPriority: u=0,i\r\nSec-Ch-Ua: \"Google Chrome\";v=\"131\",\"Chromium\";v=\"131\",\"Not_A_BRAND\";v=\"24\"\r\nSec-Ch-Ua-Mobile: ?0\r\nSec-Ch-Ua-Platform: \"Linux\"\r\nSec-Fetch-Dest: document\r\nSec-Fetch-Mode: navigate\r\nSec-Fetch-Site: same-origin\r\nSec-Fetch-User: ?1\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.36\");\r\n\r\n");
	_printRequest(req, 6);
	_checkValid(req, true, err, 6);
	return (!err) ? 0 : 1;
}

static inline void	_checkValid(const Request &req, const bool &expected, bool &err, const size_t &n) {
	std::cerr << SGR_INFO << "Request " << n << " validity: " << _validstr(req.isValid());
	std::cerr << ", expected " << _validstr(expected) << SGR_RESET << "\n";
	if (req.isValid() != expected)
		err = true;
}

static inline void	_multiAppend(Request &req) {
	req.append("GET datatracker.ietf.org/doc/h");
	_printRequest(req, 4);
	req.append("tml/rfc2616 HTTP/1.1\r\nContent-Length: 12\r");
	_printRequest(req, 4);
	req.append("\nAccept: tex");
	_printRequest(req, 4);
	req.append("t/html\r\nCache-Control: max-age=0\r\n\r\nHell");
	_printRequest(req, 4);
	req.append("o World!");
	_printRequest(req, 4);
}

static inline void	_chunkedPost(Request &req) {
	req.append("POST users/new_user.php HTTP/1.1\r\n");
	_printRequest(req, 5);
	req.append("Host: 127.0.0.1:80\r\nContent-Type: text/plain\r\nTransfer-Encoding: chunked\r\n");
	_printRequest(req, 5);
	req.append("Accept: text/html\r\n\r\n");
	_printRequest(req, 5);
	req.append("5\r\nname=\r\n");
	_printRequest(req, 5);
	req.append("14\r\nuser\nlast_name=name\n\r\n");
	_printRequest(req, 5);
	req.append("D\r\npassword=12");
	_printRequest(req, 5);
	req.append("34\r\n");
	_printRequest(req, 5);
	req.append("0\r\n\r\n");
	_printRequest(req, 5);
}

static inline void	_printRequest(const Request &req, const size_t &n) {
	std::cerr << "Request " << n << ":\n";
	std::cerr << "\tchunked: " << req.isChunked() << "\n";
	std::cerr << "\tparsed: " << req.isParsed() << "\n";
	std::cerr << "\tvalid: " << req.isValid() << "\n";
	std::cerr << "\tcontent-length: " << req.getContentLength() << "\n";
	if (!req.getURI().empty())
		std::cerr << "\tURI: " << req.getURI() << "\n";
	if (!req.getMethod().empty())
		std::cerr << "\tMethod: " << req.getMethod() << "\n";
	if (!req.getVersion().empty())
		std::cerr << "\tVersion: " << req.getVersion() << "\n";
	if (!req.getContentType().empty())
		std::cerr << "\tContentType: " << req.getContentType() << "\n";
	std::cerr << "\tHeaders:\n";
	for (auto field : req.getHeaderList())
		std::cerr << "\t\t'" << field.first << ": " << field.second << "'\n";
	if (!req.getBody().empty())
		_printBody(req);
}

static inline void	_printBody(const Request &req) {
	std::cerr << "\tBody {\n\t\t";
	for (char c : req.getBody()) {
		if (isprint(c))
			std::cerr << c;
		else if (c == '\r')
			std::cerr << "\\r";
		else if (c == '\n')
			std::cerr << "\n\t\t";
		else if (c && c < ' ')
			std::cerr << '^' << static_cast<char>(c + '@');
	}
	std::cerr << "\n\t}\n";
}
