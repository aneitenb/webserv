// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<Response.hpp>> -- <<Aida, Ilmari, Milica>>

#pragma once

#include "CommonFunctions.hpp"
#include "Request.hpp"
#include "config/ServerBlock.hpp"
#include "config/LocationBlock.hpp"

class Response {
private:
	int									_statusCode;
	std::string							_statusMessage;
	std::map<std::string, std::string>	_headers;
	std::string							_body;
	
	Request& 		_request;
	ServerBlock*	_serverBlock;
	LocationBlock*	_locationBlock;
	
	std::map<std::string, std::string> _mimeTypes;
	
	void initializeMimeTypes();

	
public:
	Response(Request& request, ServerBlock* serverBlock);
	~Response();

	void processRequest();
	void send(int clientSocket);

	int getStatusCode() const;
	const std::string& getBody() const;
	
	void setStatusCode(int code);
	void setHeader(const std::string& key, const std::string& value);
	void setBody(const std::string& body);
	void setContentType(const std::string& path);
};