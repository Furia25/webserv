/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 15:03:13 by antoine           #+#    #+#             */
/*   Updated: 2026/05/05 12:47:57 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/Request.hpp"

#define _IS_ONE_MO_ 1048576

Request::Request(Method m, const std::string& p, const std::string& q, 
                 const std::string& proto, size_t cl, 
                 const HashMap<std::string, std::string>& h)
    : method(m), path(p), query_string(q), protocol(proto), 
      content_length(cl), headers(h)
{
}

Request::~Request()
{
}

void    Request::reserveBody(size_t size)
{
    if (size < _IS_ONE_MO_)
        this->body.reserve(size);
}

void    Request::appendToBody(const uint8_t* data, size_t size)
{
    if (size > 0)
        this->body.insert(this->body.end(), data, data + size);
}

size_t    Request::getBodySize() const
{
    return this->body.size();
}

Method  Request::getMethod() const
{
    return (method);
}

const std::string&  Request::getPath() const
{
    return (path);
}

const std::string&  Request::getQueryString() const
{
    return (query_string);
}

const std::string&  Request::getProtocol() const
{
    return (protocol);
}

size_t    Request::getContentLength() const
{
    return (content_length);
}

const HashMap<std::string, std::string>&    Request::getHeaders() const
{
    return (headers);
}

size_t    Request::isLessThanOneMO() const
{
    return (this->content_length < _IS_ONE_MO_);
}

const std::vector<uint8_t>& Request::getBody() const
{
    return this->body;
}