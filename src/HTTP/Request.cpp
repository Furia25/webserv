/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 15:03:13 by antoine           #+#    #+#             */
/*   Updated: 2026/05/16 02:32:09 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/Request.hpp"

Request::Request() : method(Method::GET),
		path("/"),
		query_string(""),
		protocol(""),
		content_length(0),
		is_chunked(false),
		keep_alive(false)
{}

const std::string	*Request::operator[](const std::string &key) const
{
	Headers::const_iterator it = this->headers.find(key);
	return it == this->headers.end() ? NULL : &it->second;
}

const std::string	*Request::operator[](const char *key) const
{
	Headers::const_iterator it = this->headers.find(key);
	return it == this->headers.end() ? NULL : &it->second;
}

const std::string	*Request::operator[](const std::string &key) const
{
	Headers::const_iterator it = this->headers.find(key);
	return it == this->headers.end() ? NULL : &it->second;
}

const std::string	*Request::operator[](const char *key) const
{
	Headers::const_iterator it = this->headers.find(key);
	return it == this->headers.end() ? NULL : &it->second;
}

const std::string	*Request::operator[](const std::string &key) const
{
	Headers::const_iterator it = this->headers.find(key);
	return it == this->headers.end() ? NULL : &it->second;
}

const std::string	*Request::operator[](const char *key) const
{
	Headers::const_iterator it = this->headers.find(key);
	return it == this->headers.end() ? NULL : &it->second;
}

void Request::setCookies(const Cookies &cookies) { this->cookies = cookies; }
void Request::setHeaders(const Headers& headers) { this->headers = headers; }

const Headers&	Request::getHeaders() const 
{
	return headers;
}

Headers&	Request::getHeaders() 
{
	return headers;
}

const Cookies&	Request::getCookies() const { return cookies; }
Cookies&	Request::getCookies() { return cookies; }
