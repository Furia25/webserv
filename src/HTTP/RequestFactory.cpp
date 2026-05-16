/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestFactory.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 15:27:34 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/16 20:17:20 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/RequestFactory.hpp"
# include <algorithm>
# include "HTTP/HTTPTypes.hpp"
# include "HTTP/Utils/FileWriter.hpp"
# include "Utils/IntegerUtils.hpp"

RequestFactory::RequestFactory() 
	: is_parsing_complete(false), is_header_parsed(false), is_validated(false), is_chunked(false)
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
	if (!is_header_parsed && (raw_buffer.size() + length > MAX_HEADER_SIZE))
        throw std::overflow_error("Header size exceeded MAX_HEADER_SIZE");
	raw_buffer.insert(raw_buffer.end(), fragment, fragment + length);
	if (!is_header_parsed)
	{
		size_t header_end = findHeaderEnd();
		if (header_end != std::string::npos)
		{
			parseAllHeaders(raw_buffer, header_end);
			is_header_parsed = true;
		}
	}

}

size_t RequestFactory::findHeaderEnd()
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

size_t RequestFactory::findNewline(const std::vector<uint8_t>& buffer, size_t start, size_t max)
{
	for (size_t i = start; i < max && i < buffer.size() - 1; ++i)
		if (buffer[i] == '\r' && buffer[i+1] == '\n') return i;
	return std::string::npos;
}

void RequestFactory::parseAllHeaders(const std::vector<uint8_t>& buffer, size_t pos)
{
	size_t start = 0;
	size_t end = findNewline(buffer, start, pos);
	
	if (end != std::string::npos)
	{
		std::string first_line(buffer.begin() + start, buffer.begin() + end);
		parseRequestLine(first_line);
		start = end + 2;

		while ((end = findNewline(buffer, start, pos)) != std::string::npos && end > start)
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
	if (line.empty())
		return ;
	size_t colon_pos = line.find(':');
	if (colon_pos != std::string::npos)
	{
		std::string key = line.substr(0, colon_pos);
		std::string value = line.substr(colon_pos + 1);
		toLowerCase(key);
		size_t start = value.find_first_not_of(" \t");
		if (start != std::string::npos)
			value = value.substr(start);
		headers.insert(key, value);
	}
	else
		throw std::invalid_argument("Malformed header line: missing colon");
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
	// NOT NECESSARY
	if (protocol != HTTP_VERSION)
		throw HTTPException(HTTPCode::HTTP_VERSION_NOT_SUPPORTED);
}

void RequestFactory::validatePath()
{
	if (request_path.empty() || request_path[0] != '/')
		throw HTTPException(HTTPCode::BAD_REQUEST);
}

void RequestFactory::validateHeader() const
{
	Headers::const_iterator it = headers.find(HEADER_HOST);
	if (it == headers.end())
	{
		std::cout << "nique ta mére" << std::endl;
		throw HTTPException(HTTPCode::BAD_REQUEST);
	}
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
	for (Headers::const_iterator it = this->headers.begin(); it != this->headers.end(); ++it)
		std::cout << it->first << ":  [" << it->second << "]" << "\n";
	std::cout << "Headers Count: " << headers.size() << '\n';
	std::cout << "Complete: " << (is_validated ? "YES" : "NO") << '\n';
	std::cout << "---------------------" << std::endl;
}

std::vector<uint8_t> RequestFactory::getExtraData()
{
	size_t header_end = findHeaderEnd();

	if	(header_end == std::string::npos)
		return std::vector<uint8_t>();

	size_t body_start = header_end + 4;

	if	(body_start >= raw_buffer.size())
		return std::vector<uint8_t>();

	std::vector<uint8_t> extra(raw_buffer.begin() + body_start, raw_buffer.end());

	raw_buffer.clear();
	return extra;
}

const bool& RequestFactory::getCompleteStatus() const { return is_parsing_complete; }

const bool& RequestFactory::get_header_parsed() const { return is_header_parsed; }

const bool& RequestFactory::isValidated() const { return is_validated; }

void RequestFactory::setValidateStatus(int status) { is_validated = status; }

const std::string *RequestFactory::getHeader(const std::string& key) const
{
	Headers::const_iterator it = this->headers.find(key);
	return it != this->headers.end() ? &it->second : NULL;
}

static void	handleContentLength(Request& request, const std::string& val)
{
	try { request.content_length = IntegerUtils::strtoul_safe(val); }
	catch (...)
	{
		request.content_length = 0;
		throw HTTPException(HTTPCode::BAD_REQUEST);
	}
}

static void handleTransferEncoding(Request& request, const std::string& val)
{
	request.is_chunked = val == "chunked";
}

static void handleConnection(Request& request, const std::string& val)
{
	request.keep_alive = val == "keep-alive";
}

static std::string trim(const std::string& s)
{
	size_t start = s.find_first_not_of(" \t");
	if (start == std::string::npos) return "";
	size_t end = s.find_last_not_of(" \t");
	return s.substr(start, end - start + 1);
}

static void handleCookie(Request& request, const std::string& val)
{
	Cookies&	request_cookies = request.getCookies();
	size_t				pos = 0;

	while (pos < val.size())
	{
		size_t sep = val.find('=', pos);
		if (sep == std::string::npos)
			break;

		std::string	key = trim(val.substr(pos, sep - pos));
		size_t		end = val.find(';', sep);
		std::string	value = (end == std::string::npos)
			? trim(val.substr(sep + 1))
			: trim(val.substr(sep + 1, end - sep - 1));

		if (!key.empty())
			request_cookies.insert(key, value);

		pos = (end == std::string::npos) ? val.size() : end + 1;
	}
}

typedef void (*HeaderHandler)(Request&, const std::string&);
typedef HashMap<std::string, HeaderHandler> HandlerMap;

static HandlerMap buildHandlerMap()
{
	HandlerMap m;
	m.insert(HEADER_CONTENT_LENGTH, &handleContentLength);
	m.insert(HEADER_COOKIE, &handleCookie);
	m.insert(HEADER_TRANSFER_ENCODING, &handleTransferEncoding);
	m.insert(HEADER_CONNECTION, &handleConnection);
	return m;
}

Request	RequestFactory::build() const
{
	static const HandlerMap handlers = buildHandlerMap();
	Request result;

	try { result.method = Method::from(this->method); }
	catch (const std::domain_error& e)
	{
		result.method = Method::UNKNOWN;
	}

	result.path				= this->request_path;
	result.query_string		= this->query_string;
	result.protocol			= this->protocol;
	result.setHeaders(this->headers);

	for (Headers::const_iterator it = this->headers.begin(); it != this->headers.end(); ++it)
	{
		HandlerMap::const_iterator h = handlers.find(it->first);
		if (h != handlers.end())
			(h->second)(result, it->second);
	}
	return result;
}
