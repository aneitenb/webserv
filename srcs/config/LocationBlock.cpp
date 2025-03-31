/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationBlock.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aneitenb <aneitenb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/24 15:09:18 by aneitenb          #+#    #+#             */
/*   Updated: 2025/03/31 14:07:49 by aneitenb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "config/LocationBlock.hpp"

LocationBlock::LocationBlock() : 
	redirect(std::make_pair(0, "")),
	autoindex(false),
	allowed_methods(0) {
}

LocationBlock::~LocationBlock() {
}

LocationBlock::LocationBlock(const LocationBlock& other) :
	redirect(other.redirect),
	autoindex(other.autoindex),
	cgi_pass(other.cgi_pass),
	cgi_param(other.cgi_param),
	allowed_methods(other.allowed_methods),
	upload_store(other.upload_store),
	alias(other.alias),
	index(other.index) {
}

void LocationBlock::setAllowedMethods(uint8_t methods) {
	allowed_methods = methods;
}

bool LocationBlock::isMethodAllowed(HttpMethod method) const {
	return (allowed_methods & method) != 0;
}

void LocationBlock::addAllowedMethod(HttpMethod method) {
	allowed_methods |= method;
}

void LocationBlock::removeAllowedMethod(HttpMethod method) {
	allowed_methods &= ~method;
}

bool LocationBlock::hasRedirect() const {
	return redirect.first != 0 && !redirect.second.empty();
}

bool LocationBlock::hasCgiPass() const {
	return !cgi_pass.empty();
}

bool LocationBlock::hasUploadStore() const {
	return !upload_store.empty();
}

bool LocationBlock::hasAlias() const {
	return !alias.empty();
}

void LocationBlock::clear() {
	redirect = std::make_pair(0, "");
	autoindex = false;
	cgi_pass = "";
	cgi_param.clear();
	allowed_methods = 0;
	upload_store = "";
	alias = "";
	index = "";
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
