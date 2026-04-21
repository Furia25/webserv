/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 15:03:13 by antoine           #+#    #+#             */
/*   Updated: 2026/04/21 16:01:36 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/Request.hpp"

Request::Request(Method m, const std::string& p, const std::string& q, 
                 const std::string& proto, size_t cl, 
                 const HashMap<std::string, std::string>& h, 
                 const std::vector<uint8_t>& b)
    : method(m), path(p), query_string(q), protocol(proto), 
      content_length(cl), headers(h), body(b)
{
}

Request::~Request()
{
}

Method Request::getMethod() const
{
    return (method);
}

const std::string& Request::getPath() const
{
    return (path);
}

const std::string& Request::getQueryString() const
{
    return (query_string);
}

const std::string& Request::getProtocol() const
{
    return (protocol);
}

size_t Request::getContentLength() const
{
    return (content_length);
}

const HashMap<std::string, std::string>& Request::getHeaders() const
{
    return (headers);
}

const std::vector<uint8_t>& Request::getBody() const
{
    return (body);
}
