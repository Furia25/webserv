/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antoine <antoine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 15:59:45 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/14 17:06:45 by antoine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/Response.hpp"
# include "HTTP/HttpTypes.hpp"
# include "Utils/IntegerUtils.hpp"

Response::Response(HTTPCode code) : statusCode(code)
{
		this->setHeader("Server", SERV_NAME "/" SERV_VERSION);
}

Response::~Response()
{}

void Response::setStatusCode(HTTPCode code)
{
	this->statusCode = code;
}

void Response::setHeader(const std::string& key, const std::string& value)
{
	this->headers.insert(key, value);
}

void Response::setBody(const std::string& body)
{
	this->body = body;
	this->setContentLength(body.size());
}

void Response::setKeepAlive(bool keepAlive)
{
	if (keepAlive)
		this->setHeader("Connection", "keep-alive");
	else
		this->setHeader("Connection", "close");
}

void Response::setContentType(MIME mime_type)
{
	this->setHeader("Content-Type", MIME::toString(mime_type));
}

void Response::setContentLength(size_t length)
{
	this->setHeader("Content-Length", IntegerUtils::itoa(length));
}

std::string	Response::buildStatusLine() const
{
	return "HTTP/1.1 " + IntegerUtils::itoa(this->statusCode) + " " + HTTPCode::toString(this->statusCode) + "\r\n";
}

std::string Response::build() const
{
	return this->buildHeadersOnly() + this->body;
}

void	Response::addCookies(const std::string& cookie)
{
	this->cookies.push_back(cookie);
}

std::string Response::buildHeadersOnly() const
{
	std::string result = this->buildStatusLine();
	
	for (HashMap<std::string, std::string>::const_iterator it = this->headers.begin(); it != this->headers.end(); ++it)
		result += it->first + ": " + it->second + "\r\n";

	for (size_t i = 0; i < this->cookies.size(); ++i) 
		result += "Set-Cookie: " + this->cookies[i] + "\r\n";
		
	result += "\r\n";
	
	return result;
}

std::string Response::sendChunk(const std::string& body)
{
	std::stringstream ss;
	ss << std::hex << body.size();

	std::string chunk = ss.str() + "\r\n" + body + "\r\n";
	return chunk;
}

std::string Response::sendEndChunks()
{
	return std::string("0\r\n\r\n");
}
