/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antoine <antoine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 15:03:13 by antoine           #+#    #+#             */
/*   Updated: 2026/05/14 16:58:55 by antoine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/Request.hpp"

Request::Request() : method(Method::GET),
		path("/"),
		query_string(""),
		protocol(""),
		content_length(0),
		is_chunked(false)
{}

void Request::setCookies(const Cookies& cookies) { this->cookies = cookies; }
void Request::setHeaders(const Headers& headers) { this->headers = headers; }

const Request::Headers&	Request::getHeaders() const 
{
	return headers;
}

bool	Request::wantsKeepAlive()const
{
	if (this->headers.contain("connection"))
	{
		std::string value = this->headers.at("connection");
		if (value.find("close") != std::string::npos)
			return false;
		if (value.find("keep-alive") != std::string::npos)
			return true;
	}
	if (this->protocol == "HTTP/1.1")
		return true;
	return false;
}

Request::Headers&	Request::getHeaders() 
{
	return headers;
}

const Request::Cookies&	Request::getCookies() const { return cookies; }
Request::Cookies&	Request::getCookies() { return cookies; }
