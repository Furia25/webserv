/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 15:03:13 by antoine           #+#    #+#             */
/*   Updated: 2026/03/20 14:28:45 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"
#include <algorithm>
#include <iterator>

#define PROTOCOL "HTTP"
#define _MAX_BODY_SIZE_ 10485760

static const std::string allowed_method[] = {"GET", "POST", "DELETE"};

enum	Method
{
	GET,
	POST,
	METHOD,
	UNKNOWN,
};

Request::Request() : method(""), request_path(""), protocol(""),
	is_complete(false), header_parsed(false), content_length(0)
{
}

Request::Request(const Request &other) : method(other.method),
	request_path(other.request_path), protocol(other.protocol),
	headers(other.headers), body(other.body), is_complete(other.is_complete),
	header_parsed(other.header_parsed), content_length(other.content_length)
{
}

Request &Request::operator=(const Request &other)
{
	if (this != &other)
	{
		method = other.method;
		request_path = other.request_path;
		protocol = other.protocol;
		headers = other.headers;
		body = other.body;
		is_complete = other.is_complete;
	}
	return (*this);
}

Request::~Request()
{
}

void Request::feed(const uint8_t *fragment, size_t length)
{
	const char	*target = "\r\n\r\n";
	size_t		pos;

	raw_buffer.insert(raw_buffer.end(), fragment, fragment + length);
	if (!header_parsed && raw_buffer.size() > 8192)
		throw std::runtime_error("431 Request Header Fields Too Large");
	if (!header_parsed && raw_buffer.size() > 8192)
	{
		std::vector<uint8_t>::iterator it = std::search(raw_buffer.begin(),
				raw_buffer.end(), target, target + 4);
		if (it != raw_buffer.end())
		{
			pos = it - raw_buffer.begin();
			std::string all_headers(raw_buffer.begin(), raw_buffer.begin()
				+ pos);
			std::stringstream ss(all_headers);
			std::string line;
			if (std::getline(ss, line))
			{
				if (!line.empty() && line[line.size() - 1] == '\r')
					line.erase(line.size() - 1);
				parseRequestLine(line);
			}
			validateMethod();
			validatePath();
			validateProtocol();
			while (std::getline(ss, line))
			{
				if (!line.empty() && line[line.size() - 1] == '\r')
					line.erase(line.size() - 1);
				if (!line.empty())
					parseHeaderLine(line);
			}
			validateHeader();
			raw_buffer.erase(raw_buffer.begin(), raw_buffer.begin() + pos + 4);
			header_parsed = true;
		}
	}
	if (header_parsed && !is_complete)
	{
		if (raw_buffer.size() >= content_length)
		{
			body.assign(raw_buffer.begin(), raw_buffer.begin()
				+ content_length);
			is_complete = true;
		}
	}
}

void Request::validateHeader()
{
	long long	val;

	if (headers.find("Host") == headers.end())
		throw std::runtime_error("400 Bad Request: Missing Host header");
	if (headers.count("Content-Length"))
	{
		val = std::atoll(headers["Content-Length"].c_str());
		if (val < 0)
			throw std::runtime_error("400 Bad Request: Negative Content-Length");
		content_length = static_cast<size_t>(val);
	}
	else
	{
		content_length = 0;
		if (method == "POST")
			throw std::runtime_error("411 Length Required");
	}
	if (content_length > _MAX_BODY_SIZE_)
		throw std::runtime_error("413 Content too large");
}

void Request::parseRequestLine(std::string line)
{
	std::stringstream ss(line);
	std::string extra;
	ss >> method;
	ss >> request_path;
	ss >> protocol;
	if (method.empty() || request_path.empty() || protocol.empty())
		throw std::runtime_error("400 Bad Request: missing elements");
	if (ss >> extra)
		throw std::runtime_error("400 Bad Request: Too many elements in request line");
	if (!protocol.empty() && protocol[protocol.size() - 1] == '\r')
	{
		protocol.erase(protocol.size() - 1);
	}
}

std::string Request::trim(const std::string &str) const
{
	size_t first = str.find_first_not_of(" \t");
	if (first == std::string::npos)
		return ("");
	size_t last = str.find_last_not_of(" \t");
	return (str.substr(first, (last - first + 1)));
}

void Request::parseHeaderLine(std::string line)
{
	size_t	colon;

	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);
	colon = line.find(':');
	if (colon != std::string::npos)
	{
		std::string key = line.substr(0, colon);
		std::string value = line.substr(colon + 1);
		headers[trim(key)] = trim(value);
	}
}

void Request::validateMethod() const
{
	for (size_t i = 0; i < UNKNOWN; i++)
	{
		if (method == allowed_method[i])
			return ;
	}
	throw std::runtime_error("405 Method Not Allowed");
}

void Request::validateProtocol() const
{
	if (protocol.length() > 4)
	{
		if (protocol.substr(0, 4) != PROTOCOL)
			throw std::runtime_error("400 Bad Request: Invalid Protocol Name");
	}

	std::string version = protocol.substr(5);

	if (version == "0.9" || version.empty())
	{
		throw std::runtime_error("505 HTTP Version Not Supported");
	}
}

// Getters
void Request::validatePath() const
{
	if (request_path.find("/../"))
		throw std::runtime_error("400 Bad Request: Invalid Path");
	request_path.find("?");
}

const std::string &Request::getMethod() const
{
	return (method);
}
const std::string &Request::getRequestPath() const
{
	return (request_path);
}
const std::string &Request::getProtocol() const
{
	return (protocol);
}

const bool &Request::getCompleteStatus() const
{
	return (is_complete);
}
