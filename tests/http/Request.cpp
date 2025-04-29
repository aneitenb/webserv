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

static inline void	_printRequest(const Request &req, const size_t &n);

i32	main(void) {
	bool	err;
	Request	req;

	err = false;
	req.append("GET datatracker.ietf.org/doc/html/rfc2616 HTTP/1.1\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif/image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0,7\r\nAccept-Encoding: gzip,deflate,br,zstd\r\nAccept-Language: en-US,en;q=0.9\r\nCache-Control: max-age=0\r\nPriority: u=0,i\r\nSec-Ch-Ua: \"Google Chrome\";v=\"131\",\"Chromium\";v=\"131\",\"Not_A_BRAND\";v=\"24\"\r\nSec-Ch-Ua-Mobile: ?0\r\nSec-Ch-Ua-Platform: \"Linux\"\r\nSec-Fetch-Dest: document\r\nSec-Fetch-Mode: navigate\r\nSec-Fetch-Site: same-origin\r\nSec-Fetch-User: ?1\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.36\");\r\n\r\n");
	_printRequest(req, 1);
	if (!req.isValid())
		err = true;
	req.append("GET datatracker.ietf.org/doc/html/rfc2616 HTTp/1.1\r\n\r\n\r\n}");
	_printRequest(req, 2);
	if (!req.isValid())
		err = true;
	req.append("GET datatracker.ietf.org/doc/html/rfc2616 HTTP/1.1\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif/image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0,7\r\nAccept-Encoding: gzip,deflate,br,zstd\r\nAccept-Language: en-US,en;q=0.9\r\nCache-Control: max-age=0\r\nPriority: u=0,i\r\nSec-Ch-Ua: \"Google Chrome\";v=\"131\",\"Chromium\";v=\"131\",\"Not_A_BRAND\";v=\"24\"\r\nSec-Ch-Ua-Mobile: ?0\r\nSec-Ch-Ua-Platform: \"Linux\"\r\nSec-Fetch-Dest: document\r\nSec-Fetch-Mode: navigate\r\nSec-Fetch-Site: same-origin\r\nSec-Fetch-User: ?1\r\nUpgrade-Insecure-Requests: 1\r\nUser-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.36\");\r\n");
	_printRequest(req, 3);
	if (req.isValid())
		err = true;
	req.append("GET datatracker.ietf.org/doc/html/rfc2616 HTTP/1.1\r\n\r\n");
	_printRequest(req, 4);
	if (req.isValid())
		err = true;
	req.append("GET datatracker.ietf.org/doc/html/rfc2616 HTTP/1.1\r\nContent-Length: seven\r\n\r\n");
	_printRequest(req, 5);
	if (req.isValid())
		err = true;
	return (!err) ? 1 : 1;
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
		std::cerr << "\tBody {\n" << req.getBody() << "\n}\n";
}
