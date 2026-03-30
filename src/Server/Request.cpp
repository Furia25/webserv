/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 15:03:13 by antoine           #+#    #+#             */
/*   Updated: 2026/03/30 17:46:10 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils/HashMap.hpp"
#include "Server/Request.hpp"
#include <algorithm>
#include <iterator>
#include <sys/stat.h>
#include <unistd.h>

#define PROTOCOL "HTTP"
#define _MAX_BODY_SIZE_ 10485760
#define _MAX_PATH_SIZE_ 2048

enum	PathType
{
	INVALID,
	FILES,
	DIR,
};

static const std::string allowed_method[] = {"GET", "POST", "DELETE"};

enum	Method
{
	GET,
	POST,
	METHOD,
	UNKNOWN,
};

Request::Request() : parsing_is_complete(false), header_is_parsed(false), is_validated(false),
	content_length(0), method(""), request_path(""), protocol("")
{
}

Request::Request(const Request &other) : parsing_is_complete(other.parsing_is_complete),
	header_is_parsed(other.header_is_parsed), is_validated(other.is_validated),
	content_length(other.content_length), headers(other.headers), method(other.method),
	request_path(other.request_path), protocol(other.protocol), body(other.body)
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
		parsing_is_complete = other.parsing_is_complete;
	}
	return (*this);
}

Request::~Request()
{
}

size_t Request::find_header_end() 
{
    const char *target = "\r\n\r\n";
    std::vector<uint8_t>::iterator it = std::search(raw_buffer.begin(), raw_buffer.end(), target, target + 4);
    
    if (it == raw_buffer.end())
        return std::string::npos;
        
    return (it - raw_buffer.begin());
}

void	Request::parse_all_headers(const std::vector<uint8_t>& buffer, size_t pos)
{
	size_t start = 0;
	size_t end = 0;
	end = find_newline(buffer, start, pos);
	if (end != std::string::npos)
	{
		std::string line(buffer.begin() + start, buffer.begin() + end);
		if (!line.empty() && line[line.size()-1] == '\r') line.erase(line.size()-1);
    		parseRequestLine(line);
        start = end + 1;
	}
	while (start < pos) 
	{
        end = find_newline(buffer, start, pos);
        if (end == std::string::npos) 
			break;

        std::string line(buffer.begin() + start, buffer.begin() + end);
        if (!line.empty() && line[line.size()-1] == '\r') 
			line.erase(line.size()-1);
        
        if (!line.empty())
            parseHeaderLine(line);
        start = end + 1;
    }
}

size_t Request::find_newline(const std::vector<uint8_t>& buffer, size_t start, size_t max)
{
	for (size_t i = start; i < max; ++i)
		if (buffer[i] == '\n') return i;
	return std::string::npos;
}

void Request::feed(const uint8_t *fragment, size_t length)
{
	raw_buffer.insert(raw_buffer.end(), fragment, fragment + length);
	
	if (!header_is_parsed)
	{
		if (raw_buffer.size() > 8192)
			throw std::runtime_error("431 Request Header Fields Too Large");
			
		size_t pos = find_header_end();
		if (pos != std::string::npos)
		{
			parse_all_headers(raw_buffer, pos);
			raw_buffer.erase(raw_buffer.begin(), raw_buffer.begin() + pos + 4);
			header_is_parsed = true;
			if (content_length == 0)
				parsing_is_complete = true;
		}
	}
	if (header_is_parsed && !parsing_is_complete)
	{
		if (raw_buffer.size() >= content_length)
		{
			body.assign(raw_buffer.begin(), raw_buffer.begin() + content_length);
			raw_buffer.erase(raw_buffer.begin(), raw_buffer.begin() + content_length);
			parsing_is_complete = true;
		}
	}
}

void Request::toLowerCase(std::string &str)
{
	for (size_t i = 0; i < str.length(); ++i)
		str[i] = std::tolower(str[i]);
}

void Request::validateHeader()
{
	long long	val;

	HashMap<std::string,
		std::string>::iterator it = headers.find("Content-Length");
	if (headers.find("Host") == headers.end())
		throw std::runtime_error("400 Bad Request: Missing Host header");
	if (it != headers.end())
	{
		val = std::atoll((*it).second.c_str());
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

void Request::parseRequestLine(std::string &line)
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

void Request::parseHeaderLine(std::string &line)
{
	size_t	colon;
	size_t	startValue;

	if (!line.empty() && line[line.size() - 1] == '\r')
		line.erase(line.size() - 1);
	colon = line.find(':');
	if (colon == std::string::npos)
		return ;
	std::string key = line.substr(0, colon);
	std::string value = line.substr(colon + 1);
	startValue = value.find_first_not_of(" \t");
	if (startValue != std::string::npos)
		value = value.substr(startValue);
	toLowerCase(key);
	headers.insert(key, value);
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

int Request::checkPathType(const std::string &path)
{
	struct stat	buffer;
	int			status;

	status = stat(path.c_str(), &buffer);
	if (status == 0)
	{
		if (S_ISDIR(buffer.st_mode))
			return (DIR);
		if (S_ISREG(buffer.st_mode))
			return (FILES);
	}
	return (INVALID);
}

void	Request::invalidPath()
{
	if (request_path.size() > _MAX_PATH_SIZE_)
		throw std::runtime_error("414 Request-URI Too Long");
		
	if (request_path.find("..") != std::string::npos)
		throw std::runtime_error("403 Forbidden: Path security violation");
		
	if (request_path[0] != '/')
			throw std::runtime_error("400 Bad Request: Path must start with /");
}

void Request::validatePath()
{
	invalidPath();
	
	size_t q_path = request_path.find('?');
	if (q_path != std::string::npos)
	{
		query_path = request_path.substr(q_path + 1);
		request_path = request_path.substr(0, q_path);
	}
	
	if (request_path == "/")
		request_path = "/index.html";
	
	std::string full_path = "./www" + request_path;
	
	int type = checkPathType(full_path);
	if (type == INVALID)
		throw std::runtime_error("404 Not found");

	if (type == DIR)
	{
		if (full_path[full_path.size() - 1] != '/')
            full_path += "/";
        full_path += "index.html";
		if (checkPathType(full_path) != FILES)
            throw std::runtime_error("403 Forbidden: No index file");
	}
	if (access(full_path.c_str(), R_OK) != 0)
        throw std::runtime_error("403 Forbidden: Access denied");

    request_path = full_path;
}

void Request::print() const 
{
    std::cout << "--- REQUEST DEBUG ---" << std::endl;
    std::cout << "Method: [" << method << "]" << std::endl;
    std::cout << "Path:   [" << request_path << "]" << std::endl;
    std::cout << "Proto:  [" << protocol << "]" << std::endl;
    std::cout << "Query:  [" << query_path << "]" << std::endl;
    std::cout << "Content-Length: " << content_length << std::endl;
    std::cout << "Headers Count: " << headers.size() << std::endl;
    std::cout << "Body Size: " << body.size() << std::endl;
    if (!body.empty())
        std::cout << "Body (first 20 bytes): " << std::string(body.begin(), body.begin() + std::min(body.size(), (size_t)20)) << "..." << std::endl;
    std::cout << "Complete: " << (is_validated ? "YES" : "NO") << std::endl;
    std::cout << "---------------------" << std::endl;
}

void	Request::check()
{
	validateMethod();
	validateProtocol();
	validatePath();
	validateHeader();
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
	return (parsing_is_complete);
}

const bool &Request::isHeaderParsed()const
{
	return header_is_parsed;
}

const bool &Request::isValidated()const
{
	return is_validated;
}

void	Request::setValidateStatus(int status)
{
	if (status)
		this->is_validated = true;
	else
		this->is_validated = false;
}