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

	err = false;
	try {
		Request	req1("GET datatracker.ietf.org/doc/html/rfc2616 HTTP/1.1\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif/image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0,7\r\n Accept-Encoding: gzip,deflate,br,zstd\r\n Accept-Language: en-US,en;q=0.9\r\n Cache-Control: max-age=0\r\n Priority: u=0,i\r\n Sec-Ch-Ua: \"Google Chrome\";v=\"131\",\"Chromium\";v=\"131\",\"Not_A_BRAND\";v=\"24\"\r\n Sec-Ch-Ua-Mobile: ?0\r\n Sec-Ch-Ua-Platform: \"Linux\"\r\n Sec-Fetch-Dest: document\r\n Sec-Fetch-Mode: navigate\r\n Sec-Fetch-Site: same-origin\r\n Sec-Fetch-User: ?1\r\n Upgrade-Insecure-Requests: 1\r\n User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.36\");\r\n\r\n");
		_printRequest(req1, 0);
	} catch (std::exception &e) { err = true; std::cerr << "Request 1: " << e.what() << "\n"; }
	try {
		Request	req2("GET datatracker.ietf.org/doc/html/rfc2616 HTTP/1.1\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif/image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0,7\r\n Accept-Encoding: gzip,deflate,br,zstd\r\n Accept-Language: en-US,en;q=0.9\r\n Cache-Control: max-age=0\r\n Priority: u=0,i\r\n Sec-Ch-Ua: \"Google Chrome\";v=\"131\",\"Chromium\";v=\"131\",\"Not_A_BRAND\";v=\"24\"\r\n Sec-Ch-Ua-Mobile: ?0\r\n Sec-Ch-Ua-Platform: \"Linux\"\r\n Sec-Fetch-Dest: document\r\n Sec-Fetch-Mode: navigate\r\n Sec-Fetch-Site: same-origin\r\n Sec-Fetch-User: ?1\r\n Upgrade-Insecure-Requests: 1\r\n User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/131.0.0.0 Safari/537.36\");\r\n");
		_printRequest(req2, 2);
		err = true;
	} catch (std::exception &e) { std::cerr << "Request 2: " << e.what() << "\n"; }
	try {
		Request	req3("GET datatracker.ietf.org/doc/html/rfc2616 HTTP/1.1\r\n\r\n\r\n");
		_printRequest(req3, 3);
	} catch (std::exception &e) { err = true; std::cerr << "Request 3: " << e.what() << "\n"; }
	try {
		Request	req4("GET datatracker.ietf.org/doc/html/rfc2616 HTTP/1.1\r\n");
		_printRequest(req4, 4);
		err = true;
	} catch (std::exception &e) { std::cerr << "Request 4: " << e.what() << "\n"; }
	try {
		Request	req5("GET datatracker.ietf.org/doc/html/rfc2616 HTTP/1.1\r\nContent-Length: seven\r\n\r\n");
		_printRequest(req5, 5);
		err = true;
	} catch (std::exception &e) { std::cerr << "Request 5: " << e.what() << "\n"; }
	return (!err) ? 0 : 1;
}

static inline void	_printRequest(const Request &req, const size_t &n) {
	std::cerr << "Request " << n << ":\n";
	std::cerr << "\tchunked: " << req.isChunked() << "\n";
	std::cerr << "\tparsed: " << req.isParsed() << "\n";
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
