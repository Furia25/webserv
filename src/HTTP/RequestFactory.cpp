/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestFactory.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 15:27:34 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/11 01:44:50 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/RequestFactory.hpp"
# include <algorithm>
# include "HTTP/HttpTypes.hpp"
# include "Utils/FileWriter.hpp"
# include "Utils/IntegerUtils.hpp"

RequestFactory::RequestFactory() 
	: is_parsing_complete(false), is_header_parsed(false), is_validated(false)
{
}

RequestFactory::RequestFactory(const RequestFactory &other)
{
	*this = other;
}

RequestFactory &RequestFactory::operator=(const RequestFactory &other)
{
	if (this != &other)
	{
		this->raw_buffer = other.raw_buffer;
		this->is_parsing_complete = other.is_parsing_complete;
		this->is_header_parsed = other.is_header_parsed;
		this->is_validated = other.is_validated;
		this->headers = other.headers;
		this->method = other.method;
		this->request_path = other.request_path;
		this->query_string = other.query_string;
		this->protocol = other.protocol;
	}
	return (*this);
}

RequestFactory::~RequestFactory() {}

void RequestFactory::reset()
{
	this->raw_buffer.clear();
	this->is_parsing_complete = false;
	this->is_header_parsed = false;
	this->is_validated = true;
	this->headers.clear();
	this->method.clear();
	this->request_path.clear();
	this->query_string.clear();
	this->protocol.clear();
}

void RequestFactory::feed(const uint8_t *fragment, size_t length)
{
	raw_buffer.insert(raw_buffer.end(), fragment, fragment + length);
	
	if (!is_header_parsed)
	{
		size_t header_end = find_header_end();
		if (header_end != std::string::npos)
		{
			parse_all_headers(raw_buffer, header_end);
			is_header_parsed = true;
		}
	}

}

size_t RequestFactory::find_header_end()
{
	if (raw_buffer.size() < 4) return std::string::npos;
	for (size_t i = 0; i <= raw_buffer.size() - 4; ++i)
	{
		if (raw_buffer[i] == '\r' && raw_buffer[i+1] == '\n' && 
			raw_buffer[i+2] == '\r' && raw_buffer[i+3] == '\n')
			return i;
	}
	return std::string::npos;
}

size_t RequestFactory::find_newline(const std::vector<uint8_t>& buffer, size_t start, size_t max)
{
	for (size_t i = start; i < max && i < buffer.size() - 1; ++i)
		if (buffer[i] == '\r' && buffer[i+1] == '\n') return i;
	return std::string::npos;
}

void RequestFactory::parse_all_headers(const std::vector<uint8_t>& buffer, size_t pos)
{
	size_t start = 0;
	size_t end = find_newline(buffer, start, pos);
	
	if (end != std::string::npos)
	{
		std::string first_line(buffer.begin() + start, buffer.begin() + end);
		parseRequestLine(first_line);
		start = end + 2;

		while ((end = find_newline(buffer, start, pos)) != std::string::npos && end > start)
		{
			std::string header_line(buffer.begin() + start, buffer.begin() + end);
			parseHeaderLine(header_line);
			start = end + 2;
		}
	}
}

void RequestFactory::parseRequestLine(std::string &line)
{
	std::istringstream iss(line);
	iss >> method >> request_path >> protocol;
	
	size_t query_pos = request_path.find('?');
	if (query_pos != std::string::npos)
	{
		query_string = request_path.substr(query_pos + 1);
		request_path = request_path.substr(0, query_pos);
	}
}

void RequestFactory::parseHeaderLine(std::string &line)
{
	size_t colon_pos = line.find(':');
	if (colon_pos != std::string::npos) {
		std::string key = line.substr(0, colon_pos);
		std::string value = line.substr(colon_pos + 1);
		toLowerCase(key);
		size_t start = value.find_first_not_of(" \t");
		if (start != std::string::npos) {
			value = value.substr(start);
		}
		headers.insert(key, value);
	}
}

void RequestFactory::toLowerCase(std::string &str)
{
	for (size_t i = 0; i < str.length(); ++i)
		str[i] = std::tolower(str[i]);
}

void RequestFactory::validateMethod() const
{
	if (method.empty())
		throw HTTPException(HTTPCode::BAD_REQUEST); 
}

void RequestFactory::validateProtocol() const
{
	if (protocol != HTTP_VERSION)
		throw HTTPException(HTTPCode::HTTP_VERSION_NOT_SUPPORTED);
}

void RequestFactory::validatePath()
{
	if (request_path.empty() || request_path[0] != '/')
		throw HTTPException(HTTPCode::BAD_REQUEST);
}

void RequestFactory::validateHeader()
{
	HashMap<std::string, std::string>::iterator it = headers.find(HEADER_HOST);
	if (it == headers.end())
		throw HTTPException(HTTPCode::BAD_REQUEST);
}

void RequestFactory::invalidPath()
{
	throw HTTPException(HTTPCode::BAD_REQUEST);
}

void RequestFactory::check()
{
	validateMethod();
	validateProtocol();
	validatePath();
	validateHeader();
	is_validated = true;
}

void RequestFactory::print() const 
{
	std::cout << "--- REQUEST DEBUG ---" << '\n';
	std::cout << "Method: [" << method << "]" << '\n';
	std::cout << "Path:   [" << request_path << "]" << '\n';
	std::cout << "Proto:  [" << protocol << "]" << '\n';
	std::cout << "Query:  [" << query_string << "]" << '\n';
	for (Request::Headers::const_iterator it = this->headers.begin(); it != this->headers.end(); ++it)
		std::cout << it->first << ":  [" << it->second << "]" << "\n";
	std::cout << "Headers Count: " << headers.size() << '\n';
	std::cout << "Complete: " << (is_validated ? "YES" : "NO") << '\n';
	std::cout << "---------------------" << std::endl;
}

std::vector<uint8_t> RequestFactory::getExtraData()
{
	size_t header_end = find_header_end();

	if	(header_end == std::string::npos)
		return std::vector<uint8_t>();

	size_t body_start = header_end + 4;

	if	(body_start >= raw_buffer.size())
		return std::vector<uint8_t>();

	std::vector<uint8_t> extra(raw_buffer.begin() + body_start, raw_buffer.end());

	raw_buffer.clear();
	return extra;
}

const bool &RequestFactory::getCompleteStatus() const { return is_parsing_complete; }
const bool &RequestFactory::isHeaderParsed() const { return is_header_parsed; }
const bool &RequestFactory::isValidated() const { return is_validated; }
void RequestFactory::setValidateStatus(int status) { is_validated = status; }

const std::string *RequestFactory::getHeader(const std::string& key) const
{
	Request::Headers::const_iterator it = this->headers.find(key);
	return it != this->headers.end() ? &it->second : NULL;
}

static void	handleContentLength(Request &request, const std::string& val)
{
	try { request.content_length = IntegerUtils::strtoul_safe(val); }
	catch (...)
	{
		request.content_length = 0;
		throw HTTPException(HTTPCode::BAD_REQUEST);
	}
}

static void	handleCookie(Request& request, const std::string& val)
{
	(void) request;
	(void) val;
}

typedef void (*HeaderHandler)(Request&, const std::string&);
typedef HashMap<std::string, HeaderHandler> HandlerMap;

static HandlerMap buildHandlerMap()
{
	HandlerMap m;
	m.insert(HEADER_CONTENT_LENGTH, &handleContentLength);
	m.insert(HEADER_COOKIE, &handleCookie);
	return m;
}

Request RequestFactory::build() const
{
	static const HandlerMap handlers = buildHandlerMap();

	Request	result;

	Method rmethod = Method::UNKNOWN;
	try { rmethod = Method::from(this->method); }
	catch (const std::domain_error& e) {}

	for (Request::Headers::const_iterator it = this->headers.begin(); it != this->headers.end(); ++it)
	{
		HandlerMap::const_iterator h = handlers.find(it->first);
		if (h != handlers.end())
			(h->second)(result, it->second);
	}

	result.method = rmethod;
	result.path = this->request_path;
	result.query_string = this->query_string;
	result.protocol = this->protocol;
	result.setHeaders(this->headers);
	return result;
}
