// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<Response.cpp>> -- <<Aida, Ilmari, Milica>>

#include "http/Response.hpp"
#include "utils/message.hpp"

#undef Info
#define Info(msg)	(info(std::stringstream("") << msg, COLOR_RESPONSE))

static const std::map<int, std::string> statusMessages = {
	{200, "OK"},
	{201, "Created"},
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

Response::Response(Response&& other) noexcept {
	_statusCode = other._statusCode;
	_statusMessage = other._statusMessage;
	_headers = other._headers;
	_body = other._body;
	_fullResponse = other._fullResponse;
	_bytesSent = other._bytesSent;
	_request = other._request;
	other._request = nullptr;
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
		_fullResponse = other._fullResponse;
		_bytesSent = other._bytesSent;
		_request = other._request;
		other._request = nullptr;
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

void	Response::_printResponseInfo(void) {
	auto msg = statusMessages.find(_statusCode);

	info("\nResponse prepared:", COLOR_RESPONSE);
	info("  Version:        HTTP/1.1", COLOR_RESPONSE);
	Info("  Status code:    " << _statusCode);
	Info("  Status message: " << ((msg != statusMessages.end()) ? msg->second : "Unknown"));
#ifdef __DEBUG_RES_SHOW_HEADERS
	info("\n  Headers: ", COLOR_RESPONSE);
	for (const auto &field : _headers)
		Info("    " << field.first << ": " << field.second);
#endif /* __DEBUG_RES_SHOW_HEADERS */
#ifdef __DEBUG_RES_SHOW_BODY
	if (_body.size() != 0) {
		info("\n  Body: ", COLOR_RESPONSE);
		printBody(_headers["Content-Type"], _body, COLOR_RESPONSE);
	}
#endif /* __DEBUG_RES_SHOW_BODY */
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
	_printResponseInfo();
	_fullResponse = getStatusLine() + getHeadersString() + _body;
	_bytesSent = 0;
}

void	Response::errorResponse(const u16 statusCode) {
	this->_statusCode = statusCode;
	this->setHeader("Content-Type", "text/html");
	this->setHeader("Date", getCurrentDate());
	switch (statusCode) {
		case HTTP_REQUEST_TIMEOUT:
			this->setHeader("Connection", "close");
			break ;
	}
	this->setBody(this->getErrorPage(statusCode));
	_printResponseInfo();
	this->_fullResponse = this->getStatusLine() + this->getHeadersString() + this->_body;
	_bytesSent = 0;
}

void Response::handleResponse() {
	std::string uri = _request->getURI();
	std::string matchedLocation = findMatchingLocation(uri);
	
	if (matchedLocation.empty()){
		_locationBlock = NULL;}
	else{
		_locationBlock = &_serverBlock->getLocationBlockRef(matchedLocation);
	}

	setHeader("Date", getCurrentDate());
	if (!_request->isValid()){
		_statusCode = _request->getErrorCode();
		setBody(getErrorPage(_request->getErrorCode()));
		setHeader("Content-Type", "text/html");
		return;
	}

	// bool delayedRedirect = false;
	std::pair<int, std::string> redirect;
	
	// if (_locationBlock && _locationBlock->hasRedirect()) {
	// 	redirect = _locationBlock->getRedirect();
		
	// 	if (_request->getMethod() == "POST") {
	// 		delayedRedirect = true;
	// 	} else {
	// 		// for non-POST requests, handle redirect immediately
	// 		_statusCode = redirect.first;
	// 		setHeader("Location", redirect.second);
	// 		setBody("");
	// 		setHeader("Content-Type", "text/html");
	// 		return;
	// 	}
	// }

	if (_locationBlock && _locationBlock->hasRedirect()) {
		redirect = _locationBlock->getRedirect();
		_statusCode = redirect.first;
		setHeader("Location", redirect.second);
		setBody("");
		setHeader("Content-Type", "text/html");
		return;
	}
	
	//check for unsupported methods first
	const std::string& method = _request->getMethod();
	if (method != "GET" && method != "POST" && method != "DELETE") {
		_statusCode = 501;
		setBody(getErrorPage(501));
		setHeader("Content-Type", "text/html");
		return;
	}
	// Check if method is allowed for this location SECOND
	if (!isMethodAllowed()) {
		setMethodNotAllowedResponse();
		return;
	}
	if (method == "GET") {
		handleGet();
	} else if (method == "POST") {
		handlePost();
		// if (delayedRedirect && (_statusCode == 200 || _statusCode == 201)) {
		// 	_statusCode = redirect.first;
		// 	setHeader("Location", redirect.second);
		// 	setBody("");
		// }
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
			
			// handle relative or absolute paths
			std::string resolvedAlias;
			if (!alias.empty() && alias[0] == '/'){
				resolvedAlias = alias;	//use absolute alias
			} else {
				std::string serverRoot = _serverBlock->getRoot();
                resolvedAlias = serverRoot.empty() ? alias : serverRoot + "/" + alias;
			}
			//combine resolved alias with relative path
			if (!resolvedAlias.empty() && resolvedAlias[resolvedAlias.length() -1] != '/' &&
				!relativePath.empty() && relativePath[0] != '/') {
					return resolvedAlias + "/" + relativePath;
				}
			std::string finalPath = resolvedAlias + relativePath;
			return finalPath;
		}
	}
	
	// If location block has a root directive, use it
	std::string root;
	if (_locationBlock && _locationBlock->hasRoot()) {
		std::string locRoot = _locationBlock->getRoot();
		
		//if its absolute, use as is
		if (!locRoot.empty() && locRoot[0] == '/'){
			root = locRoot;
		}
		else {
			std::string serverRoot = _serverBlock->getRoot();
			if (serverRoot.empty()) {
				root = locRoot;
			} else {
				root = serverRoot + "/" + locRoot; //relative path
			}
		} 
	}
	else {
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
	
	bool autoindexEnabled = false;

	if (_locationBlock && _locationBlock->hasAutoindex()) {
		autoindexEnabled = _locationBlock->getAutoindex();
	} else if (_serverBlock->hasAutoindex()){
		autoindexEnabled = _serverBlock->getAutoindex();
	}

	if (autoindexEnabled) {
		generateDirectoryListing(dirPath);
		return;
	}
	
	// No index and no directory listing
	_statusCode = 403;
	setBody(getErrorPage(403));
	setHeader("Content-Type", "text/html");
}

std::string Response::findIndexFile(const std::string& dirPath) {
	std::string pathWithSlash = dirPath;
	if (!pathWithSlash.empty() && pathWithSlash[pathWithSlash.length()-1] != '/') {
		pathWithSlash += '/';
	}

	std::string indexFilename;
	
	if (_locationBlock && _locationBlock->hasIndex()) {
		indexFilename = _locationBlock->getIndex();
	} else {
		indexFilename = _serverBlock->getIndex();
	}
	if (indexFilename.empty()) {
		return "";
	}

	std::string indexPath = pathWithSlash + indexFilename;

	if (fileExists(indexPath) && hasReadPermission(indexPath)){
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
	listing << "<title>Index of " << _request->getURI() << "</title>\n";
	listing << "<style>\n";
	listing << "table { border-collapse: collapse; width: 100%; }\n";
	listing << "th, td { text-align: left; padding: 8px 15px; }\n";  // Add padding
	listing << ".file-date { min-width: 150px; }\n";  // Minimum width for date column
	listing << ".file-size { min-width: 80px; }\n";   // Minimum width for size column
	listing << "</style>\n";
	listing << "</head>\n<body>\n";
	
	//start the table
	listing << "<table>\n";
	listing << "  <tr>\n";
	listing << "    <th>Name</th>\n";
	listing << "    <th>Last Modified</th>\n";
	listing << "    <th>Size</th>\n";
	listing << "  </tr>\n";
	
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
	
	//add parent directory link
	if (_request->getURI() != "/") {
		listing << "  <tr>\n";
		listing << "    <td><a href=\"..\">Back to Home</a></td>\n";
		listing << "    <td></td>\n";
		listing << "    <td></td>\n";
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
	if (!_request->isParsed()) {
		_statusCode = 400;
		setBody(getErrorPage(400));
		setHeader("Content-Type", "text/html");
		return;
	}

	std::string uri = _request->getURI();
	
	bool isMultipart = isMultipartRequest();
	
	// only reject directory paths for non-multipart requests 
	// (files can't be posted to a directory without an explicit name)
	if (!isMultipart && (uri.empty() || uri.back() == '/' || directoryExists(resolvePath(uri)))) {
		_statusCode = 400;
		setBody(getErrorPage(400));
		setHeader("Content-Type", "text/html");
		return;
	}
	std::string path = resolveUploadPath();
	
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
	if (isMultipartRequest()) {
		std::string uploadDir = path.substr(0, path.find_last_of('/'));
		handleMultipartPost(uploadDir);
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

std::string Response::processRequestBody() {
	const std::string& rawBody = _request->getBody();
	
	std::string contentType;
	try {
		contentType = _request->getHeader("Content-Type");
	} catch (...) {
		// if there's no Content-Type header, treat as raw data
		return rawBody;
	}
	
	if (contentType.find("application/x-www-form-urlencoded") != std::string::npos) {
		// we need to decode it
		return decodeFormData(rawBody);
	}
	return rawBody;
}

std::string Response::decodeFormData(const std::string& formBody) {
	// in simple form data like "field=value", extract just the value
	size_t equalPos = formBody.find('=');
	if (equalPos == std::string::npos) {
		// if there's no '=' found, return as it is, it might be malformed
		return urlDecode(formBody);
	}
	
	// get everything after the first '='
	std::string encoded = formBody.substr(equalPos + 1);
	
	// handle multiple fields: take only the first one
	//ex: "name=John+Doe&email=john%40example.com&age=25"
	size_t ampPos = encoded.find('&');
	if (ampPos != std::string::npos) {
		encoded = encoded.substr(0, ampPos);
	}
	
	return urlDecode(encoded);
}

std::string Response::urlDecode(const std::string& encoded) {
	std::string decoded;
	
	for (size_t i = 0; i < encoded.length(); ++i) {
		if (encoded[i] == '+') {
			decoded += ' ';  // '+' becomes space in form data
		}
		else if (encoded[i] == '%' && i + 2 < encoded.length()) {
			// Decode %XX sequences (message=Hello+World%21)
			std::string hex = encoded.substr(i + 1, 2);	//gets the XX into hex
			try {
				char c = static_cast<char>(std::stoi(hex, 0, 16));	//convert hex to char(hex base is 16)
				decoded += c;
				i += 2;  // skip the XX part
			} catch (...) {
				// invalid hex, keep original
				decoded += encoded[i];
			}
		}
		else {
			decoded += encoded[i];
		}
	}
	return decoded;
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
	
	std::string dataToWrite = processRequestBody();

	file.write(dataToWrite.c_str(), dataToWrite.size());
	file.close();

	if (fileExisted) {
		_statusCode = 200;
	} else {
		setHeader("Location", _request->getURI());
		_statusCode = 201;
	}

	std::stringstream response;
	response << "<!DOCTYPE html>\n<html>\n<head>\n";
	response << "<title>Upload Successful</title>\n";
	response << "</head>\n<body>\n";
	response << "<h1>Upload Successful!</h1>\n";
	response << "<p><a href=\"/uploads/\">View all uploads</a></p>\n";
	response << "<p><a href=\"/\">Back to Home</a></p>\n";
	response << "</body>\n</html>";

	setBody(response.str());
}

/********************************************
*			MULTIPART FUNCTIONS				*
*********************************************/

bool Response::isMultipartRequest() const {
	std::string contentType = _request->getHeader("Content-Type");
	return (contentType.find("multipart/form-data") != std::string::npos);
}

std::string Response::extractBoundary(const std::string& contentType) const {
	size_t pos = contentType.find("boundary=");
	if (pos == std::string::npos) {
		return "";
	}

	std::string boundary = contentType.substr(pos + 9);	// "boundary=" is 9 chars

	return boundary;
}

std::vector<Response::MultipartFile> Response::parseMultipartData(const std::string& boundary) {
	std::vector<MultipartFile> files;
	const std::string& requestBody = _request->getBody();
	
	// Full boundary in the body has two extra --
	std::string fullBoundary = "--" + boundary;
	std::string endBoundary = fullBoundary + "--";
	
	size_t pos = 0;
	while (true) {
		// find start of a part
		pos = requestBody.find(fullBoundary, pos);
		if (pos == std::string::npos) {
			break;
		}
		pos += fullBoundary.length();
		
		// check if we've reached the end boundary
		if (requestBody.substr(pos, 2) == "--") {
			break;
		}
		
		// skip the CRLF after the boundary!!!!
		pos += 2;
		
		// Find the Content-Disposition header
		size_t headerEnd = pos;
		std::string contentDisposition;
		
		while (true) {
			size_t lineEnd = requestBody.find("\r\n", headerEnd);
			if (lineEnd == std::string::npos) {
				break;
			}
			std::string headerLine = requestBody.substr(headerEnd, lineEnd - headerEnd);
			headerEnd = lineEnd + 2; // Skip CRLF
			if (headerLine.empty()) {
				break; // empty line = end of headers
			}
			// only look for Content-Disposition header
			if (headerLine.find("Content-Disposition:") == 0) {
				contentDisposition = headerLine;
			}
		}
		
		// find the start of the next boundary to determine content length
		size_t nextBoundary = requestBody.find(fullBoundary, headerEnd);
		if (nextBoundary == std::string::npos) {
			nextBoundary = requestBody.find(endBoundary, headerEnd);
			if (nextBoundary == std::string::npos) {
				break; // malformed data
			}
		}
		
		// save content between headerEnd and nextBoundary
		size_t contentLength = nextBoundary - headerEnd - 2; // -2 for CRLF before boundary
		std::string content = requestBody.substr(headerEnd, contentLength);
		
		// save filename if present
		size_t filenamePos = contentDisposition.find("filename=\"");
		if (filenamePos != std::string::npos) {
			filenamePos += 10; // "filename="" is 10 chars
			size_t filenameEnd = contentDisposition.find("\"", filenamePos);
			if (filenameEnd != std::string::npos) {
				std::string filename = contentDisposition.substr(filenamePos, filenameEnd - filenamePos);
				
				//only add to files if there's a filename
				if (!filename.empty()) {
					MultipartFile file;
					file.filename = filename;
					file.content = content;
					files.push_back(file);
				}
			}
		}
		// move position to the start of the next part
		pos = nextBoundary;
	}
	return files;
}

void Response::handleMultipartPost(const std::string& uploadDir) {	
	std::string contentType = _request->getHeader("Content-Type");
	std::string boundary = extractBoundary(contentType);
	
	if (boundary.empty()) {
		_statusCode = 400;
		setBody(getErrorPage(400));
		setHeader("Content-Type", "text/html");
		return;
	}
	
	std::string bodyPreview = _request->getBody().substr(0, std::min((size_t)500, _request->getBody().size()));

	std::vector<MultipartFile> files = parseMultipartData(boundary);

	if (files.empty()) {
		_statusCode = 400;
		setBody(getErrorPage(400));
		setHeader("Content-Type", "text/html");
		return;
	}
	// create directory if it doesn't exist
	if (!directoryExists(uploadDir)) {
		if (mkdir(uploadDir.c_str(), 0755) != 0) {
			_statusCode = 500;
			setBody(getErrorPage(500));
			setHeader("Content-Type", "text/html");
			return;
		}
	}
	
	bool allFilesSucceeded = true;
	bool anyNewFilesCreated = false;
	std::vector<std::string> savedFiles;
	std::vector<std::string> updatedFiles;
	
	// process each file in the multipart struct
	for (const auto& file : files) {
		// unsafe: ../../../etc/psswd	safe saved as: psswd.
		std::string safeFilename = file.filename;
		size_t lastSlash = safeFilename.find_last_of("/\\");
		if (lastSlash != std::string::npos) {
			safeFilename = safeFilename.substr(lastSlash + 1);
		}
		
		// create complete path
		std::string filePath = uploadDir;
		if (filePath[filePath.length() - 1] != '/') {
			filePath += '/';
		}
		filePath += safeFilename;

		if (!hasWritePermission(uploadDir)) {
			_statusCode = 403;
			setBody(getErrorPage(403));
			setHeader("Content-Type", "text/html");
			return;
		}

		bool fileExisted = fileExists(filePath);

		std::ofstream outFile(filePath.c_str(), std::ios::binary);
		if (!outFile.is_open()) {
			allFilesSucceeded = false;
			continue;
		}
		
		outFile.write(file.content.c_str(), file.content.size());
		outFile.close();
		
		if (fileExisted) {
			updatedFiles.push_back(safeFilename);
		} else {
			savedFiles.push_back(safeFilename);
			anyNewFilesCreated = true;
		}
	}
	
	if (!allFilesSucceeded) {
		_statusCode = 500;
		setBody(getErrorPage(500));
		setHeader("Content-Type", "text/html");
		return;
	}
	
	if (anyNewFilesCreated)
		_statusCode = 201;
	else
		_statusCode = 200;
	
	std::stringstream response;
	response << "<!DOCTYPE html>\n<html>\n<head>\n";
	response << "<title>Upload Successful</title>\n";
	response << "</head>\n<body>\n";
	response << "<h1>Upload Successful!</h1>\n";
	response << "<p><a href=\"/uploads/\">View all uploads</a></p>\n";
	response << "<p><a href=\"/\">Back to Home</a></p>\n";
	response << "</body>\n</html>";
	
	setBody(response.str());
	setHeader("Content-Type", "text/html");
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
	std::string errorPage = _serverBlock->getRoot() + _serverBlock->getErrorPage(statusCode);
	
	// Try original path first
	if (!errorPage.empty() && fileExists(errorPage) && hasReadPermission(errorPage)) {
		// Read the file content
		std::ifstream file(errorPage.c_str());
		if (file.is_open()) {
			std::stringstream buffer;
			buffer << file.rdbuf();
			file.close();
			return buffer.str();
		}
	}
	
	// if og path fails and it's absolute, try relative path
	if (!errorPage.empty() && errorPage[0] == '/') {
		std::string relativePath = errorPage.substr(1); // Remove leading /
		if (fileExists(relativePath) && hasReadPermission(relativePath)) {
			std::ifstream file(relativePath.c_str());
			if (file.is_open()) {
				std::stringstream buffer;
				buffer << file.rdbuf();
				file.close();
				return buffer.str();
			}
		}
	}
	
	// default HTML if both paths fail
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

bool Response::hasExecPermission(const std::string& path) const {
	return access(path.c_str(), X_OK) == 0;
}

bool Response::hasWritePermission(const std::string& path) const {
	std::string dir = path.substr(0, path.find_last_of('/'));
	return access(dir.c_str(), W_OK) == 0;
}

void Response::handleCgi(const CGIHandler &CGI) {
	setHeader("Date", getCurrentDate());
	setHeader("Content-Type", "text/html");
	if (CGI.isValid()) {
		const std::string	&rawData = CGI.getResponseData();
		std::string			bodySection;

		// find header/body separator
		size_t headerEnd = rawData.find("\r\n\r\n");
		if (headerEnd != std::string::npos) {
			headerEnd += 4;

			std::string headerSection = rawData.substr(0, headerEnd - 2);
			bodySection = rawData.substr(headerEnd);

			// Parse headers
			std::istringstream headerStream(headerSection);
			std::string line;
			_statusCode = 200;

			while (std::getline(headerStream, line)) {
				if (!line.empty() && line.back() == '\r') {
					line.pop_back(); // removes \r
				}

				if (line.empty()) break; //end of headers if empty line

				size_t colonPos = line.find(':');
				if (colonPos == std::string::npos) continue; // skip malformed lines

				std::string headerName = line.substr(0, colonPos);
				std::string headerValue = line.substr(colonPos + 1);

				// trim whitespace
				size_t start = headerValue.find_first_not_of(" \t");
				if (start != std::string::npos) {
					size_t end = headerValue.find_last_not_of(" \t");
					headerValue = headerValue.substr(start, end - start + 1);
				}

				if (headerName == "Status") {
					size_t spacePos = headerValue.find(' ');
					std::string statusStr = (spacePos != std::string::npos) ? headerValue.substr(0, spacePos) : headerValue;
					try {
						_statusCode = std::stoi(statusStr);
					} catch (...) {
						_statusCode = 200;
					}
				} else {
					setHeader(headerName, headerValue);
				}
			}
		} else
			bodySection = rawData;
		setBody(bodySection);
	} else {
		_statusCode = CGI.getErrorCode();
		setBody(getErrorPage(_statusCode));
	}
	_printResponseInfo();
	_fullResponse = getStatusLine() + getHeadersString() + _body;
	_bytesSent = 0;
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

void Response::setStatusCode(const int code){
	_statusCode = code;
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

