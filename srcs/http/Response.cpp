// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<Response.cpp>> -- <<Aida, Ilmari, Milica>>

#include "http/Response.hpp"

static const std::map<int, std::string> statusMessages = {
	{200, "OK"},
	{201, "Created"},	//do we need these?
	{204, "No Content"},//needed?
	{301, "Moved Permanently"},
	{302, "Found"},	//needed?
	{400, "Bad Request"},
	{403, "Forbidden"},
	{404, "Not Found"},
	{405, "Method Not Allowed"},
	{408, "Request Timeout"},
	{409, "Conflict"},
	{410, "Gone"},
	{411, "Length Required"},
	{413, "Payload Too Large"},
	{414, "URI Too Long"},
	{431, "Request Header Fields Too Large"},
	{500, "Internal Server Error"},
	{501, "Not Implemented"},
	{503, "Service Unavailable"},
	{505, "HTTP Version Not Supported"}
};

Response::Response(Request& request, ServerBlock* serverBlock)
	: _statusCode(200),  _bytesSentSoFar(0), _totalMsgBytes(0), \
	 _request(request), _serverBlock(serverBlock), _locationBlock(NULL)
{
	initializeMimeTypes();
	std::cout << "RESPONSE MADE\n";
}

Response::Response(){}

Response::~Response() {}

Response::Response(Response&& other) noexcept{
	_statusCode = other._statusCode;
	_statusMessage = other._statusMessage;
	_headers = other._headers;
	_body = other._body;
	_bytesSentSoFar = other._bytesSentSoFar;
	_totalMsgBytes = other._totalMsgBytes;
	_serverBlock = other._serverBlock;
	other._serverBlock = nullptr;
	_locationBlock = other._locationBlock;
	other._locationBlock = nullptr;
	_mimeTypes = other._mimeTypes;
}

Response& Response::operator=(Response&& other) noexcept{
	if (this != &other){
		_statusCode = other._statusCode;
		_statusMessage = other._statusMessage;
		_headers = other._headers;
		_body = other._body;
		_bytesSentSoFar = other._bytesSentSoFar;
		_totalMsgBytes = other._totalMsgBytes;
		_serverBlock = other._serverBlock;
		other._serverBlock = nullptr;
		_locationBlock = other._locationBlock;
		other._locationBlock = nullptr;
		_mimeTypes = other._mimeTypes;		
	}
	return (*this);
}

void Response::initializeMimeTypes() {
	_mimeTypes = {
		{".html", "text/html"},
		{".htm", "text/html"},
		{".txt", "text/plain"},
		{".css", "text/css"},
		{".js", "application/javascript"},
		{".json", "application/json"},
		{".xml", "application/xml"},
		{".png", "image/png"},
		{".jpg", "image/jpeg"},
		{".jpeg", "image/jpeg"},
		{".gif", "image/gif"},
		{".svg", "image/svg+xml"},
		{".ico", "image/x-icon"},
		{".pdf", "application/pdf"},
		{".zip", "application/zip"},
		{".mp3", "audio/mpeg"},
		{".mp4", "video/mp4"}
	};
}

std::string Response::getMimeType(const std::string& path) const {
	size_t pos = path.find_last_of('.');
	if (pos == std::string::npos)
		return "application/octet-stream";	//default MIME type
	
	std::string extension = path.substr(pos);
	auto it = _mimeTypes.find(extension);
	if (it != _mimeTypes.end())
		return it->second;
	
	return "application/octet-stream";
}

void Response::processRequest() {
	std::string uri = _request.getURI();
	_locationBlock = &_serverBlock->getLocationBlockRef(uri);
	
	if (!isMethodAllowed()) {
		_statusCode = 405;
		_body = getErrorPage(405);
		setHeader("Allow", _locationBlock->allowedMethodsToString());
		return;
	}
	
	if (_locationBlock->hasRedirect()) {
		const auto& redirect = _locationBlock->getRedirect();
		_statusCode = redirect.first;
		setHeader("Location", redirect.second);
		_body = "";
		return;
	}
	
	const std::string& method = _request.getMethod();
	if (method == "GET") {
		handleGet();
	} else if (method == "POST") {
		handlePost();
	} else if (method == "DELETE") {
		handleDelete();
	} else {
		_statusCode = 501; // Not implemented
		_body = getErrorPage(501);
	}
}

void Response::send() {
	//WRITE
    std::cout << "Sending data\n";
}

void Response::handleGet(){
	//WRITE
    std::cout << "Getting\n";
}

void Response::handlePost(){
	//WRITE
    std::cout << "Posting\n";
}

void Response::handleDelete(){
	//WRITE
    std::cout << "Deleting\n";
}

bool Response::isMethodAllowed() const {
	if (!_locationBlock)
		return false;
	
	const std::string& method = _request.getMethod();
	
	if (method == "GET")
		return _locationBlock->isMethodAllowed(GET);
	else if (method == "POST")
		return _locationBlock->isMethodAllowed(POST);
	else if (method == "DELETE")
		return _locationBlock->isMethodAllowed(DELETE);
	
	return false;
}

std::string Response::getErrorPage(int statusCode) const {
	std::string errorPage = _serverBlock->getErrorPage(statusCode);
	if (!errorPage.empty() && fileExists(errorPage) && hasReadPermission(errorPage))
		return errorPage;
	
	//make default error page if it doesn't exist or have permissions
	std::stringstream ss;
	ss << "<!DOCTYPE html>\n<html>\n<head>\n<title>Error " << statusCode << "</title>\n</head>\n<body>\n";
	ss << "<h1>Error " << statusCode << "</h1>\n";
	
	auto it = statusMessages.find(statusCode);
	if (it != statusMessages.end())
		ss << "<p>" << it->second << "</p>\n";
	
	ss << "</body>\n</html>";
	return ss.str();
}

//st_mode is stat struct member containign file type and permissions
bool Response::fileExists(const std::string& path) const {
	struct stat buffer;
	return (stat(path.c_str(), &buffer) == 0 && S_ISREG(buffer.st_mode));
}

//access() returns 0 when permission exists, return true(1) if it has permissions
bool Response::hasReadPermission(const std::string& path) const {
	return access(path.c_str(), R_OK) == 0;
}

std::string Response::getStatusLine() const {
	std::stringstream ss;
	ss << "HTTP/1.1 " << _statusCode << " ";
	
	auto it = statusMessages.find(_statusCode);
	if (it != statusMessages.end())
		ss << it->second;
	else
		ss << "Unknown";
	
	ss << "\r\n";
	return ss.str();
}

//EDIT THIS TO INCLUDE DATE 
std::string Response::getHeadersString() const {
	std::stringstream ss;
	
	for (const auto& header : _headers)	//iterates through the _headers map
		ss << header.first << ": " << header.second << "\r\n";
	
	ss << "\r\n"; //empty line separates headers from body
	return ss.str();
}

int Response::getStatusCode() const {
	return _statusCode;
}

const std::string& Response::getBody() const {
	return _body;
}

void Response::setStatusCode(int code) {
	_statusCode = code;
}

void Response::setHeader(const std::string& key, const std::string& value) {
	_headers[key] = value;
}

void Response::setBody(const std::string& body) {
	_body = body;
	setHeader("Content-Length", std::to_string(_body.size()));
}

void Response::setContentType(const std::string& path) {
	setHeader("Content-Type", getMimeType(path));
}


//newly added 
void Response::addToBytesSent(ssize_t adding){
	_bytesSentSoFar += adding;
}

bool Response::allSent(){
	if (_totalMsgBytes == _bytesSentSoFar)
		return true;
	return false;
}

const std::string& Response::getRawData() const{
	return (_rawData);
}

ssize_t Response::getBytes() const{
	return (_bytesSentSoFar);
}
