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

Response::Response(const Request& request, ServerBlock* serverBlock)
	: _statusCode(200), 
	_request(request), 
	_serverBlock(serverBlock), 
	_locationBlock(NULL),
	_bytesSent(0),
{
	initializeMimeTypes();
}

Response::~Response() {}

Response::clear() {
	_statusCode = 200;
	_headers.clear();
	_body.clear();
	_fullResponse.clear();
	_bytesSent = 0;
	_locationBlock = NULL;
	_request.clear();	//implement a clear in Request class
}

void Response::setRequest(const Request& request) {
	_request = request;
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

void Response::handleResponse() {
	std::string uri = _request.getURI();
	std::string matchedLocation = findMatchingLocation(uri);
	
	if (matchedLocation.empty())
		_locationBlock = NULL;
	else
		_locationBlock = &_serverBlock->getLocationBlockRef(matchedLocation);

	setHeader("Date", getCurrentDate());
	
	if (!isMethodAllowed()) {
		_statusCode = 405;
		setBody(getErrorPage(405));
		setHeader("Allow", _locationBlock->allowedMethodsToString());
		setHeader("Content-Type", "text/html");
		return;
	}
	
	if (_locationBlock->hasRedirect()) {
		const auto& redirect = _locationBlock->getRedirect();
		_statusCode = redirect.first;
		setHeader("Location", redirect.second);
		setBody("");
		setHeader("Content-Type", "text/html");
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
		setBody(getErrorPage(501));
		setHeader("Content-Type", "text/html");
	}
}

void  Response::prepareResponse() {
	_fullResponse = getStatusLine() + getHeadersString() + _body;
	_bytesSent = 0;
}

bool Response::isComplete() const {
	return (_bytesSent >= _fullResponse.size());
}

void Response::handleGet(){
	std::string path = resolvePath(_request.getURI());

	if (isCgiRequest(path)) {
		//handleCgi(path);
		return;
	}

	std::string absPath = path;
	if (!absPath.empty() && absPath[absPath.length()-1] == '/' && 
		absPath.length() > 1)
		absPath = absPath.substr(0, absPath.length()-1);

 	if (directoryExists(absPath))
	{
		std::string indexFile;
		if (_locationBlock && _locationBlock->hasIndex()) {
			indexFile = _locationBlock->getIndex();
		} else {
			indexFile = _serverBlock->getIndex();
		}
		
		// path has to end with slash for appending index file
		if (!absPath.empty() && absPath[absPath.length()-1] != '/') {
			absPath += '/';
		}
		
		std::string indexPath = absPath + indexFile;
		if (fileExists(indexPath) && hasReadPermission(indexPath)) {
			readFile(indexPath);
			return;
		}
		
		// No directory listing and no index file
		_statusCode = 403;
		setBody(getErrorPage(403));
		setHeader("Content-Type", "text/html");
		return;
	}
	//check if path is a file
	if (fileExists(absPath) && hasReadPermission(absPath)) {
		readFile(absPath);
		return;
	}
	//file not found
	_statusCode = 404;
	setBody(getErrorPage(404));
	setHeader("Content-Type", "text/html");
}

void Response::handlePost(){
	std::string path;

	//check for upload_store directive
	if (_locationBlock && _locationBlock->hasUploadStore()) {
		std::string filename = _request.getURI();
		size_t lastSlash = filename.find_last_of('/');
		if (lastSlash != std::string::npos) {
			filename = filename.substr(lastSlash + 1);
		}
		
		// making sure upload dir ends with a slash
		std::string uploadDir = _locationBlock->getUploadStore();
		if (!uploadDir.empty() && uploadDir[uploadDir.length()-1] != '/') {
			uploadDir += '/';
		}
		
		path = uploadDir + filename;
	} 
	else {
		path = resolvePath(_request.getURI());
	}

	std::string dirPath = path.substr(0, path.find_last_of('/'));
	if (!directoryExists(dirPath)) {
		_statusCode = 404;
		setBody(getErrorPage(404));
		setHeader("Content-Type", "text/html");
		return;
	}
	
	if (!hasWritePermission(path)) {
		_statusCode = 403;
		setBody(getErrorPage(403));
		setHeader("Content-Type", "text/html");
		return;
	}
	
	if (_request.getContentLength() > _serverBlock->getClientMaxBodySize()) {
		_statusCode = 413;
		setBody(getErrorPage(413));
		setHeader("Content-Type", "text/html");
		return;
	}
	
	bool fileExisted = fileExists(path);
	std::ofstream file(path, std::ios::binary | std::ios::trunc);
	if (!file.is_open()) {
		_statusCode = 500;
		setBody(getErrorPage(500));
		setHeader("Content-Type", "text/html");
		return;
	}
	
	file.write(_request.getBody().c_str(), _request.getBody().size());
	file.close();
	
	if (fileExisted)
		_statusCode = 200;
	else
	{
		setHeader("Location", _request.getURI());
		_statusCode = 201;
	}
	setBody("");
}

void Response::handleDelete(){
	std::string path = _locationBlock->getRoot() + _request.getURI();
	
	// Check if file exists
	if (!fileExists(path)) {
		_statusCode = 404;
		setBody(getErrorPage(404));
		setHeader("Content-Type", "text/html");
		return;
	}
	
	// do we have permission to delete
	std::string dirPath = path.substr(0, path.find_last_of('/'));
	if (!hasWritePermission(dirPath)) {
		_statusCode = 403;
		setBody(getErrorPage(403));
		setHeader("Content-Type", "text/html");
		return;
	}
	
	if (std::remove(path.c_str()) != 0) {
		_statusCode = 500;
		setBody(getErrorPage(500));
		setHeader("Content-Type", "text/html");
		return;
	}
	
	_statusCode = 204; // success, just no content to send back
	setBody("");
}

std::string Response::resolvePath(const std::string& uri) {
	// Check if location block has an alias directive
	if (_locationBlock && _locationBlock->hasAlias()) {
		// When using alias, replace the location path with the alias path
		std::string locationPath = findMatchingLocation(uri);
		
		if (!locationPath.empty() && uri.find(locationPath) == 0) {
			std::string alias = _locationBlock->getAlias();
			std::string relativePath = uri.substr(locationPath.length());
			
			// make sure there's a slash between alias and relative path
			if (!alias.empty() && alias[alias.length()-1] != '/' && 
				!relativePath.empty() && relativePath[0] != '/') {
				return alias + "/" + relativePath;
			}
			return alias + relativePath;
		}
	}
	
	// If location block has a root directive, use it
	std::string root;
	if (_locationBlock && _locationBlock->hasRoot()) {
		root = _locationBlock->getRoot();
	} else {
		root = _serverBlock->getRoot();
	}
	
	// make sure there's a slash between root and URI
	if (!root.empty() && root[root.length()-1] != '/' && 
		!uri.empty() && uri[0] != '/') {
		return root + "/" + uri;
	}
	return root + uri;
}

std::string Response::findMatchingLocation(const std::string& uri) {
	std::string bestMatch = "";
	size_t bestMatchLength = 0;
	
	const std::map<std::string, LocationBlock>& locations = _serverBlock->getLocationBlocks();
	
	for (std::map<std::string, LocationBlock>::const_iterator it = locations.begin(); 
		 it != locations.end(); ++it) {
		std::string locationPath = it->first;
		
		// Normalize paths for comparison - ensure both start with /
		std::string absLocationPath = locationPath;
		std::string absUri = uri;
		
		if (!absLocationPath.empty() && absLocationPath[0] != '/') {
			absLocationPath = "/" + absLocationPath;
		}
		
		if (!absUri.empty() && absUri[0] != '/') {
			absUri = "/" + absUri;
		}
		
		// If this location path is a prefix of the URI and longer than our best match so far
		if (absUri.find(absLocationPath) == 0) {
			// make sure it's a complete segment match
			if (absLocationPath.length() > bestMatchLength && 
				(absUri.length() == absLocationPath.length() || 
				 absUri[absLocationPath.length()] == '/' || 
				 absLocationPath[absLocationPath.length() - 1] == '/')) {
				bestMatch = locationPath; // Store the original path
				bestMatchLength = absLocationPath.length();
			}
		}
	}
	
	return bestMatch;
}

void Response::generateDirectoryListing(const std::string& path) {
	DIR* dir;
	struct dirent* entry;
	
	dir = opendir(path.c_str());
	if (!dir) {
		_statusCode = 500;
		setBody(getErrorPage(500));
		setHeader("Content-Type", "text/html");
		return;
	}
	
	std::stringstream listing;
	listing << "<!DOCTYPE html>\n<html>\n<head>\n";
	listing << "<title>Index of " << _request.getURI() << "</title>\n";
	listing << "</head>\n<body>\n";
	listing << "<h1>Index of " << _request.getURI() << "</h1>\n";
	listing << "<hr>\n<pre>\n";
	
	// add parent directory link if not at root
	if (_request.getURI() != "/") {
		listing << "<a href=\"..\">..</a>\n";
	}
	
	while ((entry = readdir(dir)) != NULL) {
		std::string name = entry->d_name;
	
		if (name == "." || name == "..") {
			continue;
		}
		
		std::string fullPath = path + "/" + name;
		bool isDir = directoryExists(fullPath);
		
		listing << "<a href=\"" 
				<< (_request.getURI() == "/" ? "" : _request.getURI()) 
				<< "/" << name << (isDir ? "/" : "") << "\">" 
				<< name << (isDir ? "/" : "") << "</a>\n";
	}
	
	listing << "</pre>\n<hr>\n</body>\n</html>";
	closedir(dir);
	
	_statusCode = 200;
	setBody(listing.str());
	setHeader("Content-Type", "text/html");
}

void Response::readFile(const std::string& path) {
	std::ifstream file(path.c_str(), std::ios::binary);
	if (!file.is_open()) {
		_statusCode = 500;
		setBody(getErrorPage(500));
		setHeader("Content-Type", "text/html");
		return;
	}
	
	// Get file size
	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	file.seekg(0, std::ios::beg);
	
	// Read file content
	_body.resize(size);
	file.read(&_body[0], size);
	file.close();
	
	_statusCode = 200;
	setContentType(path);
	setHeader("Content-Length", std::to_string(_body.size()));
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

//st_mode is stat struct member containing file type and permissions
bool Response::fileExists(const std::string& path) const {
	struct stat buffer;
	return (stat(path.c_str(), &buffer) == 0 && S_ISREG(buffer.st_mode));
}

bool Response::directoryExists(const std::string& path) const {
	struct stat buffer;
	return (stat(path.c_str(), &buffer) == 0 && S_ISDIR(buffer.st_mode));
}

//access() returns 0 when permission exists, return true(1) if it has permissions
bool Response::hasReadPermission(const std::string& path) const {
	return access(path.c_str(), R_OK) == 0;
}

bool Response::hasWritePermission(const std::string& path) const {
	std::string dir = path.substr(0, path.find_last_of('/'));
	return access(dir.c_str(), W_OK) == 0;
}

bool Response::isCgiRequest(const std::string& path) {
	if (!_locationBlock->hasCgiPass()) {
		return false;
	}

	size_t dotPos = path.find_last_of('.');
	if (dotPos == std::string::npos) {
		return false;
	}
	return true;
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

std::string Response::getCurrentDate() const {
	char buffer[100];
	time_t now = time(0);
	struct tm* timeinfo = gmtime(&now);
	
	//format: Mon, 28 Apr 2025 08:42:03 GMT
	strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
	return std::string(buffer);
}

int Response::getStatusCode() const {
	return _statusCode;
}

const std::string& Response::getBody() const {
	return _body;
}

void Response::setHeader(const std::string& key, const std::string& value) {
	_headers[key] = value;
}

void Response::setBody(const std::string& body) {
	_body = body;
	if (_statusCode != 204)
		setHeader("Content-Length", std::to_string(_body.size()));
}

void Response::setContentType(const std::string& path) {
	setHeader("Content-Type", getMimeType(path));
}