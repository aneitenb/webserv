/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationBlock.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aneitenb <aneitenb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 13:13:30 by aneitenb          #+#    #+#             */
/*   Updated: 2025/04/07 15:35:56 by aneitenb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once
#include "Webserv.hpp"

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
	bool autoindexSet;
	std::string cgi_pass;
	std::map<std::string, std::string> cgi_param;
	uint8_t allowed_methods;
	std::string upload_store;
	std::string alias;
	std::string index;
	std::string root;

public:
	LocationBlock();
	LocationBlock(const LocationBlock& other);
	~LocationBlock();

	void clear();
	
	bool isMethodAllowed(HttpMethod method) const;
	void setAllowedMethods(uint8_t methods);
	std::string allowedMethodsToString() const;
	uint8_t getAllowedMethods() const;

	bool hasRedirect() const;
	bool hasCgiPass() const;
	bool hasUploadStore() const;
	bool hasAlias() const;
	bool hasIndex() const;
	bool hasRoot() const;
	bool hasAutoindex() const;
	bool hasAllowedMethods() const;
	bool hasCgiParam(const std::string& paramName) const;
	
	const std::pair<int, std::string>& getRedirect() const;
	void setRedirect(const std::pair<int, std::string>& redirect);
	
	std::string getRoot() const;
	void setRoot(const std::string& rootValue);
	bool getAutoindex() const;
	void setAutoindex(bool autoindex);
	std::string getCgiPass() const;
	void setCgiPass(const std::string& cgi_pass);
	const std::map<std::string, std::string>& getCgiParams() const;
	void setCgiParam(const std::string& name, const std::string& value);
	std::string getUploadStore() const;
	void setUploadStore(const std::string& upload_store);
	std::string getAlias() const;
	void setAlias(const std::string& alias);
	std::string getIndex() const;
	void setIndex(const std::string& index);
};
