#pragma once

#include "server/WebServer.hpp"

// HTTP Method enum as bit flags for easier method checking
enum HttpMethod {
	GET = 1,
	POST = 2,
	DELETE = 4,
};

class LocationBlock {
private:
	std::pair<int, std::string> redirect;
	bool autoindex;
	std::string cgi_pass;
	std::map<std::string, std::string> cgi_param;
	uint8_t allowed_methods;
	std::string upload_store;
	std::string alias;
	std::string index;

public:
	LocationBlock();
	LocationBlock(const LocationBlock& other);
	~LocationBlock();

	void setAllowedMethods(uint8_t methods);
	bool isMethodAllowed(HttpMethod method) const;
	void addAllowedMethod(HttpMethod method);
	void removeAllowedMethod(HttpMethod method);
	std::string allowedMethodsToString() const;

	bool hasRedirect() const;
	bool hasCgiPass() const;
	bool hasUploadStore() const;
	bool hasAlias() const;

	void clear();

	std::pair<int, std::string> getRedirect() const { return redirect; }
	void setRedirect(const std::pair<int, std::string>& redirect) { this->redirect = redirect; }
	
	bool getAutoindex() const { return autoindex; }
	void setAutoindex(bool autoindex) { this->autoindex = autoindex; }
	
	std::string getCgiPass() const { return cgi_pass; }
	void setCgiPass(const std::string& cgi_pass) { this->cgi_pass = cgi_pass; }
	
	std::map<std::string, std::string> getCgiParams() const { return cgi_param; }
	void addCgiParam(const std::string& name, const std::string& value) { cgi_param[name] = value; }
	
	uint8_t getAllowedMethods() const { return allowed_methods; }
	
	std::string getUploadStore() const { return upload_store; }
	void setUploadStore(const std::string& upload_store) { this->upload_store = upload_store; }
	
	std::string getAlias() const { return alias; }
	void setAlias(const std::string& alias) { this->alias = alias; }
	
	std::string getIndex() const { return index; }
	void setIndex(const std::string& index) { this->index = index; }
};