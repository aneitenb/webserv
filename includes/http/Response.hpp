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
	ssize_t								_bytesSentSoFar; //newly added
	ssize_t								_totalMsgBytes; //newly added
	std::string							_rawData;

	Request 		_request; //can't be a reference, need a default constructor
	ServerBlock*	_serverBlock; //maybe this should be VirtualHost type????????
	LocationBlock*	_locationBlock;
	
	std::map<std::string, std::string> _mimeTypes;
	
	void initializeMimeTypes();

	
public:
	Response(Request& request, ServerBlock* serverBlock);
    Response();
	~Response();
	Response(const Response &other) = delete;
	Response &operator=(const Response &other) = delete;
	// move constructor
	Response(Response &&other) noexcept;
	Response &operator=(Response &&other) noexcept;

	void processRequest();
	/*probably won't need the client socket, but will need how many bytes need to be sent
	will need to set the _rawData too so I can know where I'm at*/
	void send(int clientSocket); 
    void handleGet();
    void handlePost();
    void handleDelete();

	int addToBytesSent(ssize_t adding);
	bool allSent();
	const std::string& getRawData() const;
	ssize_t getBytes() const;


	int getStatusCode() const;
    std::string getStatusLine() const;
    std::string getMimeType(const std::string& path) const;
	const std::string& getBody() const;
    std::string getErrorPage(int statusCode) const;
    std::string getHeadersString() const;
	
	void setStatusCode(int code);
	void setHeader(const std::string& key, const std::string& value);
	void setBody(const std::string& body);
	void setContentType(const std::string& path);

    bool isMethodAllowed() const;
    bool fileExists(const std::string& path) const;
    bool hasReadPermission(const std::string& path) const;
};