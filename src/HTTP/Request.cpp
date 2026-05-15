/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 15:03:13 by antoine           #+#    #+#             */
/*   Updated: 2026/05/15 02:50:35 by vdurand          ###   ########.fr       */
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
