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

Response::Response(){}

Response::~Response() {}

Response::Response(Response&& other) noexcept
	: _request(other._request){
	_statusCode = other._statusCode;
	_statusMessage = other._statusMessage;
	_headers = other._headers;
	_body = other._body;
	_bytesSent = other._bytesSent;
	_serverBlock = other._serverBlock;
	other._serverBlock = nullptr;
	_locationBlock = other._locationBlock;
	other._locationBlock = nullptr;
	_mimeTypes = other._mimeTypes;
}

Response& Response::operator=(Response&& other) noexcept{
	if (this != &other){
		_request = other._request;
		_statusCode = other._statusCode;
		_statusMessage = other._statusMessage;
		_headers = other._headers;
		_body = other._body;
		_bytesSent = other._bytesSent;
		_serverBlock = other._serverBlock;
		other._serverBlock = nullptr;
		_locationBlock = other._locationBlock;
		other._locationBlock = nullptr;
		_mimeTypes = other._mimeTypes;		
	}
	return (*this);
}

Response::Response(Request* request, ServerBlock* serverBlock)
	: _statusCode(200), 
	_bytesSent(0),
	_request(request), 
	_serverBlock(serverBlock), 
	_locationBlock(NULL)
{
	initializeMimeTypes();
}

void Response::clear() {
	_statusCode = 200;
	_headers.clear();
	_body.clear();
	_fullResponse.clear();
	_bytesSent = 0;
	_locationBlock = NULL;
}

void Response::setRequest(Request& request) {
	_request = &request;
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

void  Response::prepareResponse() {
	handleResponse();
	_fullResponse = getStatusLine() + getHeadersString() + _body;
	_bytesSent = 0;
}

void Response::handleResponse() {
	std::string uri = _request->getURI();
	std::string matchedLocation = findMatchingLocation(uri);
	
	if (matchedLocation.empty())
		_locationBlock = NULL;
	else
		_locationBlock = &_serverBlock->getLocationBlockRef(matchedLocation);

	setHeader("Date", getCurrentDate());
	if (_request->getMethod() != "GET" && _request->getMethod() != "POST" && _request->getMethod() != "DELETE")
	{
		_statusCode = 405;
		setBody(getErrorPage(405));
		setHeader("Content-Type", "text/html");
		return;
	}
	if (!_request->isValid()){
		_statusCode = 400;
		setBody(getErrorPage(400));
		setHeader("Content-Type", "text/html");
		return;
	}
	
	bool delayedRedirect = false;
	std::pair<int, std::string> redirect;
	
	if (_locationBlock && _locationBlock->hasRedirect()) {
		redirect = _locationBlock->getRedirect();

		if (_request->getMethod() == "POST") {
			delayedRedirect = true;
		} else {
			// for non-POST requests, handle redirect immediately
			_statusCode = redirect.first;
			setHeader("Location", redirect.second);
			setBody("");
			setHeader("Content-Type", "text/html");
			return;
		}
	}
	
	const std::string& method = _request->getMethod();
	if (method == "GET") {
		handleGet();
	} else if (method == "POST") {
		handlePost();
		if (delayedRedirect && (_statusCode == 200 || _statusCode == 201)) {
			_statusCode = redirect.first;
			setHeader("Location", redirect.second);
			setBody("");
		}
	} else if (method == "DELETE") {
		handleDelete();
	} else {
		_statusCode = 501;
		setBody(getErrorPage(501));
		setHeader("Content-Type", "text/html");
	}
}

bool Response::isComplete() const {
	return (_bytesSent == _fullResponse.size());
}

/********************************************
*				GET FUNCTIONS				*
*********************************************/

void Response::handleGet() {
	std::string path = resolvePath(_request->getURI());
  
  if (isCgiRequest(path)) {   //DOES THIS WORK?
		handleCgi(path);
		return;
	}
	
	if (!resourceExists(path)) {
		return;
	}
	if (!isMethodAllowed()) {
		setMethodNotAllowedResponse();
		return;
	}
	getResource(path);
}

std::string Response::resolvePath(const std::string& uri) {
	// check if location block has an alias directive
	if (_locationBlock && _locationBlock->hasAlias()) {
		// replace the location path with the alias path
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
	
	// put slash between root and URI
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
		
		// make both start with / for comparison
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
				bestMatch = locationPath; // store the original path
				bestMatchLength = absLocationPath.length();
			}
		}
	}
	return bestMatch;
}

bool Response::resourceExists(const std::string& path) {
	std::string normalized = path;
	
	//get rid of trailing slash unless at root
	if (!normalized.empty() && normalized[normalized.length()-1] == '/' && 
		normalized.length() > 1) {
		normalized = normalized.substr(0, normalized.length()-1);
	}
	if (directoryExists(normalized) || fileExists(normalized)) {
		return true;
	}
	_statusCode = 404;
	setBody(getErrorPage(404));
	setHeader("Content-Type", "text/html");
	return false;
}

void Response::setMethodNotAllowedResponse() {
	_statusCode = 405;
	setBody(getErrorPage(405));
	
	if (_locationBlock && _locationBlock->hasAllowedMethods()) {
		setHeader("Allow", _locationBlock->allowedMethodsToString());
	} else if (_serverBlock->hasAllowedMethods()) {
		setHeader("Allow", _serverBlock->allowedMethodsToString());
	} else {
		setHeader("Allow", "");
	}
	setHeader("Content-Type", "text/html");
}

void Response::getResource(const std::string& path) {
	std::string normalizedPath = path;
	
	if (directoryExists(normalizedPath)) {
		getDirectory(normalizedPath);
	} else if (fileExists(normalizedPath)) {
		getFile(normalizedPath);
	} else {
		//shouldn't happen bc we already checked existence
		_statusCode = 404;
		setBody(getErrorPage(404));
		setHeader("Content-Type", "text/html");
	}
}

void Response::getDirectory(const std::string& dirPath) {
	std::string indexPath = findIndexFile(dirPath);
	if (!indexPath.empty()) {
		readFile(indexPath);
		return;
	}
	
	if (_locationBlock && _locationBlock->hasAutoindex()) {
		if (_locationBlock->getAutoindex()) {
			generateDirectoryListing(dirPath);
			return;
		}
	}
	
	// No index and no directory listing
	_statusCode = 403;
	setBody(getErrorPage(403));
	setHeader("Content-Type", "text/html");
}

std::string Response::findIndexFile(const std::string& dirPath) {
	std::string indexFile;
	if (_locationBlock && _locationBlock->hasIndex()) {
		indexFile = _locationBlock->getIndex();
	} else {
		indexFile = _serverBlock->getIndex();
	}
	
	//make path end with slash
	std::string pathWithSlash = dirPath;
	if (!pathWithSlash.empty() && pathWithSlash[pathWithSlash.length()-1] != '/') {
		pathWithSlash += '/';
	}
	
	std::string indexPath = pathWithSlash + indexFile;
	if (fileExists(indexPath) && hasReadPermission(indexPath)) {
		return indexPath;
	}
	
	return "";
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
	listing << "<title>Index of " << _request->getURI() << "</title>\n";
	listing << "</head>\n<body>\n";
	listing << "<h1>Index of " << _request->getURI() << "</h1>\n";
	listing << "<hr>\n";
	
	//start the table
	listing << "<table>\n";
	listing << "  <tr>\n";
	listing << "    <th>Name</th>\n";
	listing << "    <th>Last Modified</th>\n";
	listing << "    <th>Size</th>\n";
	listing << "  </tr>\n";
	
	//add parent directory link if not at root
	if (_request->getURI() != "/") {
		listing << "  <tr>\n";
		listing << "    <td><a href=\"..\">Parent Directory</a></td>\n";
		listing << "    <td></td>\n";
		listing << "    <td></td>\n";
		listing << "  </tr>\n";
	}
	
	// vector of entries for sorting. format: <filename, isDir>
	std::vector<std::pair<std::string, bool>> entries;
	
	while ((entry = readdir(dir)) != NULL) {
		std::string name = entry->d_name;
		
		if (name == "." || name == "..") {
			continue;
		}
		std::string fullPath = path + "/" + name;
		bool isDir = directoryExists(fullPath);
		
		entries.push_back(std::make_pair(name, isDir));
	}
	
	// using lambda function(anonymous function)that is defined inline
	// code between {...} contains the logic for comparing two directory entries
	// sorts entries directories first, then files alphabetically
	std::sort(entries.begin(), entries.end(), 
		[](const std::pair<std::string, bool>& entry1, const std::pair<std::string, bool>& entry2) {
			const std::string& fileName1 = entry1.first;
			const std::string& fileName2 = entry2.first;
			bool isDirectory1 = entry1.second;
			bool isDirectory2 = entry2.second;
			
			// If one is a directory and one is a file, directories come first
			if (isDirectory1 && !isDirectory2) {
				return true;
			}
			if (!isDirectory1 && isDirectory2) {
				return false; // file after directory
			}
			//both are the same type so sort alphabetically by name
			return fileName1 < fileName2;
		});
	
	// Add entries to the table
	for (size_t i = 0; i < entries.size(); i++) {
		std::string name = entries[i].first;
		bool isDir = entries[i].second;
		std::string fullPath = path + "/" + name;
		
		// get file stats for size and date
		struct stat fileStats;
		stat(fullPath.c_str(), &fileStats);
		
		// format last modified time
		char timeBuffer[80];
		struct tm* timeinfo = localtime(&fileStats.st_mtime);
		strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", timeinfo);

		std::string fileSize;
		if (!isDir) {
			if (fileStats.st_size < 1024) {
				fileSize = std::to_string(fileStats.st_size) + " B";
			} else if (fileStats.st_size < 1024 * 1024) {
				fileSize = std::to_string(fileStats.st_size / 1024) + " KB";
			} else {
				fileSize = std::to_string(fileStats.st_size / (1024 * 1024)) + " MB";
			}
		}
		
		listing << "  <tr>\n";
		listing << "    <td><a href=\"" 
				<< (_request->getURI() == "/" ? "" : _request->getURI()) 
				<< "/" << name << (isDir ? "/" : "") << "\">" 
				<< name << (isDir ? "/" : "") << "</a></td>\n";
		listing << "    <td class=\"file-date\">" << timeBuffer << "</td>\n";
		listing << "    <td class=\"file-size\">" << fileSize << "</td>\n";
		listing << "  </tr>\n";
	}
	
	listing << "</table>\n";
	listing << "<hr>\n";
	listing << "</body>\n</html>";
	closedir(dir);
	
	_statusCode = 200;
	setBody(listing.str());
	setHeader("Content-Type", "text/html");
}

void Response::getFile(const std::string& filePath) {
	if (hasReadPermission(filePath)) {
		readFile(filePath);
	} else {
		_statusCode = 403;
		setBody(getErrorPage(403));
		setHeader("Content-Type", "text/html");
	}
}

void Response::readFile(const std::string& path) {
	std::ifstream file(path.c_str(), std::ios::binary);
	if (!file.is_open()) {
		_statusCode = 500;
		setBody(getErrorPage(500));
		setHeader("Content-Type", "text/html");
		return;
	}
	
	// Get file size using seekg "seek get pointer" :
	//  - it positions the file's read pointer at a specific location in the file.
	// tellg returns the current position of the read pointer as a number of bytes
	// from the beginning of the file
	file.seekg(0, std::ios::end);
	size_t size = file.tellg();
	file.seekg(0, std::ios::beg);
	
	// Read file content and copy contents directly into _body
	// resize(size) pre-allocates memory for the string to hold exactly the size of the file
	_body.resize(size);	
	file.read(&_body[0], size);
	file.close();
	
	_statusCode = 200;
	setContentType(path);
	setHeader("Content-Length", std::to_string(_body.size()));
}


/********************************************
*				POST FUNCTIONS				*
*********************************************/

void Response::handlePost() {
	std::string path = resolveUploadPath();
  
  if (isCgiRequest(path)) {     //CHECK THIS
		handleCgi(path);
		return;
	}
	
	if (!checkDir(path)) {
		return;
	}
	if (!isMethodAllowed()) {
		setMethodNotAllowedResponse();
		return;
	}
	if (_request->getContentLength() > _serverBlock->getClientMaxBodySize()) {
		_statusCode = 413;
		setBody(getErrorPage(413));
		setHeader("Content-Type", "text/html");
		return;
	}
	postResource(path);
}

std::string Response::resolveUploadPath() {
	if (_locationBlock && _locationBlock->hasUploadStore()) {
		std::string filename = _request->getURI();
		size_t lastSlash = filename.find_last_of('/');
		if (lastSlash != std::string::npos) {
			filename = filename.substr(lastSlash + 1);	// extract filename from URI
		}
		
		std::string uploadDir = _locationBlock->getUploadStore();
		
		//if upload_store is a relative path, add server root
		if (!uploadDir.empty() && uploadDir[0] != '/') {
			uploadDir = _serverBlock->getRoot() + "/" + uploadDir;
		}
		//make upload dir end with slash
		if (!uploadDir.empty() && uploadDir[uploadDir.length()-1] != '/') {
			uploadDir += '/';
		}
		return uploadDir + filename;
	} 
	else {
		return resolvePath(_request->getURI());
	}
}

bool Response::checkDir(const std::string& path) {
	std::string dirPath = path.substr(0, path.find_last_of('/'));
	
	if (!directoryExists(dirPath)) {
		if (mkdir(dirPath.c_str(), 0755) != 0) {
			_statusCode = 404;
			setBody(getErrorPage(404));
			setHeader("Content-Type", "text/html");
			return false;
		}
	}
	if (!hasWritePermission(path)) {
		_statusCode = 403;
		setBody(getErrorPage(403));
		setHeader("Content-Type", "text/html");
		return false;
	}
	return true;
}

void Response::postResource(const std::string& path) {
	bool fileExisted = fileExists(path);
	std::ofstream file(path, std::ios::binary | std::ios::trunc);
	
	if (!file.is_open()) {
		_statusCode = 500;
		setBody(getErrorPage(500));
		setHeader("Content-Type", "text/html");
		return;
	}
	
	file.write(_request->getBody().c_str(), _request->getBody().size());
	file.close();

	if (fileExisted) {
		_statusCode = 200;
	} else {
		setHeader("Location", _request->getURI());
		_statusCode = 201;
	}
	
	// // handle redirect if needed
	// if ((_statusCode == 200 || _statusCode == 201) && _locationBlock && _locationBlock->hasRedirect()) {
	// 	std::string redirectUrl = _locationBlock->getRedirect().second;
	// 	int redirectStatus = _locationBlock->getRedirect().first;
		
	// 	setHeader("Location", redirectUrl);
	// 	_statusCode = redirectStatus;
	// }
	setBody("");
}


/************************************************
*				DELETE FUNCTIONS				*
*************************************************/

void Response::handleDelete() {
	std::string path = resolveDeletePath();
	
	if (!fileExists(path)) {
		_statusCode = 404;
		setBody(getErrorPage(404));
		setHeader("Content-Type", "text/html");
		return;
	}

	if (!isMethodAllowed()) {
		setMethodNotAllowedResponse();
		return;
	}
	
	if (!checkDeletePermissions(path)) {
		return;
	}
	
	deleteResource(path);
}

std::string Response::resolveDeletePath() {
	std::string root;
	
	if (_locationBlock && _locationBlock->hasRoot()) {
		root = _locationBlock->getRoot();
	} else {
		root = _serverBlock->getRoot();
	}
	
	// Add slash between root and URI
	if (!root.empty() && root[root.length()-1] != '/' && 
		!_request->getURI().empty() && _request->getURI()[0] != '/') {
		return root + "/" + _request->getURI();
	} else {
		return root + _request->getURI();
	}
}

bool Response::checkDeletePermissions(const std::string& path) {
	std::string dirPath = path.substr(0, path.find_last_of('/'));
	if (!hasWritePermission(dirPath)) {
		_statusCode = 403;
		setBody(getErrorPage(403));
		setHeader("Content-Type", "text/html");
		return false;
	}
	return true;
}

void Response::deleteResource(const std::string& path) {
	if (std::remove(path.c_str()) != 0) {
		_statusCode = 500;
		setBody(getErrorPage(500));
		setHeader("Content-Type", "text/html");
		return;
	}
	
	_statusCode = 204; // Success, no content
	setBody("");
}


/************************************************
*				UTILITY FUNCTIONS				*
*************************************************/

bool Response::isMethodAllowed() const {
	const std::string& method = _request->getMethod();
	HttpMethod requestMethod;
	
	if (method == "GET")
		requestMethod = GET;
	else if (method == "POST")
		requestMethod = POST;
	else if (method == "DELETE")
		requestMethod = DELETE;
	else
		return false;
	
	if (_locationBlock && _locationBlock->hasAllowedMethods()) {
		return _locationBlock->isMethodAllowed(requestMethod);
	}
	if (_serverBlock->hasAllowedMethods()) {
		return _serverBlock->isMethodAllowed(requestMethod);
	}
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

bool Response::isCgiRequest(const std::string& path) const {
    if (!_locationBlock || !_locationBlock->hasCgiPass()) {
        return false;
    }

    // checking that file extension is (.py)
    size_t dotPos = path.find_last_of('.');
    if (dotPos == std::string::npos) {
        return false;
    }
    
    std::string extension = path.substr(dotPos);
    return (extension == CGI_EXTENSION);
}

void Response::handleCgi(const std::string& path) {    
    std::string scriptPath = path;
    std::string cgiExecutable = _locationBlock->getCgiPass();
    
    if (!fileExists(scriptPath)) {
        _statusCode = 404;
        setBody(getErrorPage(404));
        setHeader("Content-Type", "text/html");
        return;
    }
    
    if (!hasReadPermission(scriptPath)) {
        _statusCode = 403;
        setBody(getErrorPage(403));
        setHeader("Content-Type", "text/html");
        return;
    }
    
    // TODO: Implement actual CGI execution
    // 1. Setting up environment variables
    // 2. Creating pipes for stdin/stdout
    // 3. Forking a process
    // 4. Executing the CGI script
    // 5. Reading the response
    // 6. Parsing headers from the response

    _statusCode = 501;
    setBody(getErrorPage(501));
    setHeader("Content-Type", "text/html");
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

void Response::addToBytesSent(ssize_t adding){
	_bytesSent += adding;
}

ssize_t Response::getBytes() const{
	return (_bytesSent);
}

std::string Response::getFullResponse() const{
	return (_fullResponse);
}
