/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antoine <antoine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 15:03:13 by antoine           #+#    #+#             */
/*   Updated: 2026/03/07 16:09:49 by antoine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

#define PROTOCOL "HTTP"

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

void Request::feed(const std::vector<uint8_t> &fragment)
{
	size_t	pos;

	raw_buffer.insert(raw_buffer.end(), fragment.begin(), fragment.end());
	if (!header_parsed)
	{
		std::string current_str(raw_buffer.begin(), raw_buffer.end());
		pos = current_str.find("\r\n\r\n");
		if (pos != std::string::npos)
		{
			std::string all_headers = current_str.substr(0, pos);
			std::stringstream ss(all_headers);
			std::string line;
			if (std::getline(ss, line))
			{
				if (!line.empty() && line[line.size() - 1] == '\r')
					line.erase(line.size() - 1);
				parseRequestLine(line);
			}
			while (std::getline(ss, line) && line != "\r" && !line.empty())
				parseHeaderLine(line);
			if (headers.find("Host") == headers.end())
				throw std::runtime_error("400 Bad Request: Missing Host header");
			raw_buffer.erase(raw_buffer.begin(), raw_buffer.begin() + pos + 4);
			if (headers.count("Content-Length"))
				content_length = std::atoll(headers["Content-Length"].c_str());
			else
				content_length = 0;
			header_parsed = true;
		}
	}
	if (header_parsed)
	{
		if (raw_buffer.size() >= content_length)
		{
			body = raw_buffer;
			is_complete = true;
		}
	}
}

void Request::parseRequestLine(std::string line)
{
	std::stringstream ss(line);
	std::string extra;
	ss >> method;       //  "GET"
	ss >> request_path; //  "/"
	ss >> protocol;     //  "HTTP/1.1"
	if (method.empty() || request_path.empty() || protocol.empty())
		throw std::runtime_error("400 Bad Request: missing elements");
	if (ss >> extra)
		throw std::runtime_error("400 Bad Request: Too many elements in request line");
	if (!protocol.empty() && protocol[protocol.size() - 1] == '\r')
	{
		protocol.erase(protocol.size() - 1);
	}
	validateMethod();
	validateProtocol();
	validatePath();
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
	if (protocol.substr(0, 4) != PROTOCOL)
		throw std::runtime_error("400 Bad Request: Invalid Protocol Name");

	std::string version = protocol.substr(5);

	if (version != "1.1" && version != "1.0")
	{
		throw std::runtime_error("505 HTTP Version Not Supported");
	}
}

// Getters
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

const bool &Request::is_complete() const
{
	return (is_complete);
}
