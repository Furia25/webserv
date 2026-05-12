/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antoine <antoine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 15:03:13 by antoine           #+#    #+#             */
/*   Updated: 2026/05/12 22:04:30 by antoine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/Request.hpp"


Request::Request() : method(Method::GET),
		path("/"),
		query_string(""),
		protocol(""),
		content_length(0),
		is_chunk_encoding(false)
{}

Request::Request(const Request& other) 
{
	this->method = other.method;
	this->path = other.path;
	this->query_string = other.query_string;
	this->protocol = other.protocol;
	this->headers = other.headers;
	this->content_length = other.content_length;
	this->is_chunk_encoding = other.is_chunk_encoding;
}

Request&	Request::operator=(const Request& other)
{
	if (this != &other) 
	{
		this->method = other.method;
		this->path = other.path;
		this->query_string = other.query_string;
		this->protocol = other.protocol;
		this->headers = other.headers;
		this->content_length = other.content_length;
		this->is_chunk_encoding = other.is_chunk_encoding;
	}
	return *this;
}

Request::~Request()
{
}

Method  Request::getMethod() const
{
    return (method);
}

const std::string&  Request::getPath() const
{
    return (path);
}

void Request::setCookies(const Cookies& cookies) { this->cookies = cookies; }
void Request::setHeaders(const Headers& headers) { this->headers = headers; }

const Request::Headers&	Request::getHeaders() const 
{
	return headers;
}
Request::Headers&	Request::getHeaders() 
{
	return headers;
}

const std::string&	Request::getQueryString() const
{
    return (query_string);
}

const std::string&	Request::getProtocol() const
{
    return (protocol);
}
const Request::Cookies&	Request::getCookies() const { return cookies; }
Request::Cookies&	Request::getCookies() { return cookies; }

bool	Request::isChunked() const 
{
	return this->is_chunk_encoding;
}

size_t	Request::getContentLength() const
{
    return (content_length);
}

