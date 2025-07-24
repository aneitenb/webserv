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

	struct MultipartFile {
		std::string filename;
		std::string content;
	};
	
	std::map<std::string, std::string> _mimeTypes;
	
	void initializeMimeTypes();
	std::string getStatusLine() const;
	std::string getHeadersString() const;
	std::string getMimeType(const std::string& path) const;
	std::string getCurrentDate() const;
	
	bool hasReadPermission(const std::string& path) const;
	bool hasWritePermission(const std::string& path) const;
	bool fileExists(const std::string& path) const;
	bool directoryExists(const std::string& path) const;
	bool isMethodAllowed() const;
	void setMethodNotAllowedResponse();
	std::string resolvePath(const std::string& uri);
	std::string findMatchingLocation(const std::string& uri);
	
	void handleGet();
	bool resourceExists(const std::string& path);
	void getResource(const std::string& path);
	void getDirectory(const std::string& dirPath);
	void getFile(const std::string& filePath);
	std::string findIndexFile(const std::string& dirPath);
	bool isDirectoryListingEnabled();
	void generateDirectoryListing(const std::string& path);
	void readFile(const std::string& path);
	
	void handlePost();
	std::string resolveUploadPath();
	bool checkDir(const std::string& path);
	std::string processRequestBody();
	std::string decodeFormData(const std::string& formBody);
	std::string urlDecode(const std::string& encoded);
	void postResource(const std::string& path);
	void handlePostRedirect();
	
	void handleDelete();
	std::string resolveDeletePath();
	bool checkDeletePermissions(const std::string& path);
	void deleteResource(const std::string& path);
	
	bool isMultipartRequest() const;
	std::vector<MultipartFile> parseMultipartData(const std::string& boundary);
	std::string extractBoundary(const std::string& contentType) const;
	void handleMultipartPost(const std::string& uploadDir);
	
	bool isCgiRequest(const std::string& path) const;
	
	void handleCgi(const std::string& path);
	
	public:
	Response(Request* request, ServerBlock* serverBlock);
	~Response();
	Response();
	Response(Response &&other) noexcept;
	Response &operator=(Response &&other) noexcept;
	void clear();
	void setRequest(Request& request);
	
	void handleResponse();
	
	int getStatusCode() const;
	const std::string& getBody() const;
	std::string getFullResponse() const;
	std::string getErrorPage(int statusCode) const;
  
	void addToBytesSent(ssize_t adding);
	ssize_t getBytes() const;
	
	void setStatusCode(const int code);
	void setHeader(const std::string& key, const std::string& value);
	void setBody(const std::string& body);
	void setContentType(const std::string& path);

	void prepareResponse();
	bool isComplete() const;
};