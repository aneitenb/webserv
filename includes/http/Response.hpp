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
	std::string							_fullResponse;
  size_t								_bytesSent;
	
	Request* 		_request;
	ServerBlock*	_serverBlock;
	LocationBlock*	_locationBlock;
	
	std::map<std::string, std::string> _mimeTypes;
	
	void initializeMimeTypes();

	std::string getStatusLine() const;
	std::string getHeadersString() const;
	std::string getMimeType(const std::string& path) const;
	std::string getErrorPage(int statusCode) const;
	std::string getCurrentDate() const;
	bool isMethodAllowed() const;
	bool hasReadPermission(const std::string& path) const;
	bool hasWritePermission(const std::string& path) const;
	bool fileExists(const std::string& path) const;
	bool directoryExists(const std::string& path) const;
	std::string resolvePath(const std::string& uri);
    bool isCgiRequest(const std::string& path);
	std::string findMatchingLocation(const std::string& uri);
    void readFile(const std::string& path);
	void generateDirectoryListing(const std::string& path);

	void handleGet();
	void handlePost();
	void handleDelete();
	
public:
	Response(Request* request, ServerBlock* serverBlock);
	~Response();
  	Response(); //add to cpp
  	Response(Response &&other) noexcept; //add
	Response &operator=(Response &&other) noexcept; //add
	void clear();
	void setRequest(Request& request);

	void handleResponse();

	int getStatusCode() const;
	const std::string& getBody() const;
	std::string getFullResponse() const;
  
	void addToBytesSent(ssize_t adding);  //check
	// bool allSent();             //check
	// const std::string& getRawData() const;  //check
	ssize_t getBytes() const;     //check
	
	void setHeader(const std::string& key, const std::string& value);
	void setBody(const std::string& body);
	void setContentType(const std::string& path);

	void prepareResponse();
	bool isComplete() const;
};