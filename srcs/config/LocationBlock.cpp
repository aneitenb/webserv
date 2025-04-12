/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationBlock.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aneitenb <aneitenb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 15:09:18 by aneitenb          #+#    #+#             */
/*   Updated: 2025/04/12 15:42:47 by aneitenb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config/LocationBlock.hpp"

LocationBlock::LocationBlock() : 
	redirect(std::make_pair(0, "")),
	autoindex(false),
	autoindexSet(false),
	allowed_methods(0) {
}

LocationBlock::~LocationBlock() {
}

LocationBlock::LocationBlock(const LocationBlock& other) :
	redirect(other.redirect),
	autoindex(other.autoindex),
	autoindexSet(other.autoindexSet),
	cgi_pass(other.cgi_pass),
	cgi_param(other.cgi_param),
	allowed_methods(other.allowed_methods),
	upload_store(other.upload_store),
	alias(other.alias),
	index(other.index),
	root(other.root) {
}

LocationBlock& LocationBlock::operator=(const LocationBlock& other) {
	if (this != &other) {
		redirect = other.redirect;
		autoindex = other.autoindex;
		autoindexSet = other.autoindexSet;
		cgi_pass = other.cgi_pass;
		cgi_param = other.cgi_param;
		allowed_methods = other.allowed_methods;
		upload_store = other.upload_store;
		alias = other.alias;
		index = other.index;
		root = other.root;
	}
	return *this;
}

void LocationBlock::clear() {
	redirect = std::make_pair(0, "");
	autoindex = false;
	autoindexSet = false;
	cgi_pass = "";
	cgi_param.clear();
	allowed_methods = 0;
	upload_store = "";
	alias = "";
	index = "";
	root = "";
}
	
std::string LocationBlock::allowedMethodsToString() const {
	std::stringstream ss;
		
	if (isMethodAllowed(GET))
		ss << "GET ";
	if (isMethodAllowed(POST))
		ss << "POST ";
	if (isMethodAllowed(DELETE))
		ss << "DELETE ";
	
	std::string result = ss.str();
	if (!result.empty() && result[result.length() - 1] == ' ')
		result = result.substr(0, result.length() - 1);
		
	return result;
}

std::string LocationBlock::getRoot() const {
	return root;
}

bool LocationBlock::hasRoot() const {
	return !root.empty();
}

void LocationBlock::setRoot(const std::string& rootValue) {
	this->root = rootValue;
}

bool LocationBlock::hasAllowedMethods() const {
	return allowed_methods != 0;
}

bool LocationBlock::isMethodAllowed(HttpMethod method) const {
	return (allowed_methods & method) != 0;
}

void LocationBlock::setAllowedMethods(uint8_t methods) {
	allowed_methods = methods;
}

uint8_t LocationBlock::getAllowedMethods() const {
	return allowed_methods;
}

bool LocationBlock::hasRedirect() const {
	return redirect.first != 0 && !redirect.second.empty();
}

const std::pair<int, std::string>& LocationBlock::getRedirect() const {
	return redirect;
}

void LocationBlock::setRedirect(const std::pair<int, std::string>& redirectPair) {
	this->redirect = redirectPair;
}

bool LocationBlock::hasCgiPass() const {
	return !cgi_pass.empty();
}

std::string LocationBlock::getCgiPass() const {
	return cgi_pass;
}

void LocationBlock::setCgiPass(const std::string& cgiPassValue) {
	this->cgi_pass = cgiPassValue;
}

bool LocationBlock::hasCgiParam(const std::string& paramName) const {
	return cgi_param.find(paramName) != cgi_param.end();
}

const std::map<std::string, std::string>& LocationBlock::getCgiParams() const {
	return cgi_param;
}

void LocationBlock::setCgiParam(const std::string& key, const std::string& value) {
	this->cgi_param[key] = value;
}

bool LocationBlock::hasUploadStore() const {
	return !upload_store.empty();
}

std::string LocationBlock::getUploadStore() const {
	return upload_store;
}

void LocationBlock::setUploadStore(const std::string& uploadStoreValue) {
	this->upload_store = uploadStoreValue;
}

bool LocationBlock::hasAlias() const {
	return !alias.empty();
}

std::string LocationBlock::getAlias() const {
	return alias;
}

void LocationBlock::setAlias(const std::string& aliasValue) {
	this->alias = aliasValue;
}

bool LocationBlock::hasAutoindex() const {
	return autoindexSet;
}

bool LocationBlock::getAutoindex() const {
	return autoindex;
}

void LocationBlock::setAutoindex(bool autoindexValue) {
	this->autoindex = autoindexValue;
	this->autoindexSet = true;
}

bool LocationBlock::hasIndex() const {
	return !index.empty();
}

std::string LocationBlock::getIndex() const {
	return index;
}

void LocationBlock::setIndex(const std::string& indexValue) {
	this->index = indexValue;
}
