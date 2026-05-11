/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 15:03:13 by antoine           #+#    #+#             */
/*   Updated: 2026/05/11 02:03:55 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/Request.hpp"

#define _IS_ONE_MO_ 1048576

void	Request::reserveBody(size_t size)
{
	if (size < _IS_ONE_MO_)
		this->body.reserve(size);
}

void	Request::appendToBody(const uint8_t* data, size_t size)
{
	if (size > 0)
		this->body.insert(this->body.end(), data, data + size);
}

size_t	Request::getBodySize() const { return this->body.size(); }

const Request::Headers&	Request::getHeaders() const { return headers; }
Request::Headers&	Request::getHeaders() { return headers; }

const Request::Cookies&	Request::getCookies() const { return cookies; }
Request::Cookies&	Request::getCookies() { return cookies; }

size_t	Request::isLessThanOneMO() const { return (this->content_length < _IS_ONE_MO_); }

void Request::setCookies(const Cookies& cookies) { this->cookies = cookies; }
void Request::setHeaders(const Headers& headers) { this->headers = headers; }

const std::vector<uint8_t> &Request::getBody() const { return this->body; }