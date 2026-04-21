/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestBuilder.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 15:27:34 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/21 16:01:31 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/RequestBuilder.hpp"
# include <algorithm>
# include "HTTP/HttpTypes.hpp"


RequestBuilder::RequestBuilder() 
    : parsing_is_complete(false), header_is_parsed(false), is_validated(false), content_length(0)
{
}

RequestBuilder::RequestBuilder(const RequestBuilder &other)
{
    *this = other;
}

RequestBuilder &RequestBuilder::operator=(const RequestBuilder &other)
{
    if (this != &other)
    {
        this->raw_buffer = other.raw_buffer;
        this->parsing_is_complete = other.parsing_is_complete;
        this->header_is_parsed = other.header_is_parsed;
        this->is_validated = other.is_validated;
        this->content_length = other.content_length;
        this->headers = other.headers;
        this->method = other.method;
        this->request_path = other.request_path;
        this->query_path = other.query_path;
        this->protocol = other.protocol;
        this->body = other.body;
    }
    return (*this);
}

RequestBuilder::~RequestBuilder()
{
}

void RequestBuilder::feed(const uint8_t *fragment, size_t length)
{
    raw_buffer.insert(raw_buffer.end(), fragment, fragment + length);

    if (!header_is_parsed)
    {
        size_t header_end = find_header_end();
        if (header_end != std::string::npos)
        {
            parse_all_headers(raw_buffer, header_end);
            header_is_parsed = true;
            
            if (raw_buffer.size() > header_end + 4)
                body.insert(body.end(), raw_buffer.begin() + header_end + 4, raw_buffer.end());
        }
    }
    else
        body.insert(body.end(), fragment, fragment + length);
    if (header_is_parsed)
    {
        if (body.size() >= content_length)
            parsing_is_complete = true;
    }
}

size_t RequestBuilder::find_header_end()
{
    if (raw_buffer.size() < 4) return std::string::npos;
    for (size_t i = 0; i <= raw_buffer.size() - 4; ++i) {
        if (raw_buffer[i] == '\r' && raw_buffer[i+1] == '\n' && 
            raw_buffer[i+2] == '\r' && raw_buffer[i+3] == '\n')
            return i;
    }
    return std::string::npos;
}

size_t RequestBuilder::find_newline(const std::vector<uint8_t>& buffer, size_t start, size_t max)
{
    for (size_t i = start; i < max && i < buffer.size() - 1; ++i) {
        if (buffer[i] == '\r' && buffer[i+1] == '\n') return i;
    }
    return std::string::npos;
}

void RequestBuilder::parse_all_headers(const std::vector<uint8_t>& buffer, size_t pos)
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

void RequestBuilder::parseRequestLine(std::string &line)
{
    std::istringstream iss(line);
    iss >> method >> request_path >> protocol;
    
    size_t query_pos = request_path.find('?');
    if (query_pos != std::string::npos) {
        query_path = request_path.substr(query_pos + 1);
        request_path = request_path.substr(0, query_pos);
    }
}

void RequestBuilder::parseHeaderLine(std::string &line)
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

        if (key == _HEADER_CONTENT_LENGTH_)
            content_length = std::strtoul(value.c_str(), NULL, 10);
        if (content_length > _DEFAULT_MAX_BODY_SIZE_)
            throw HTTPException(HTTPCode::PAYLOAD_TOO_LARGE);
    }
}

void RequestBuilder::toLowerCase(std::string &str)
{
    for (size_t i = 0; i < str.length(); ++i)
        str[i] = std::tolower(str[i]);
}

void RequestBuilder::validateMethod() const
{
    if (method.empty())
        throw HTTPException(HTTPCode::BAD_REQUEST); 
}

void RequestBuilder::validateProtocol() const
{
    if (protocol != _HTTP_VERSION_)
        throw HTTPException(HTTPCode::_HTTP_VERSION__NOT_SUPPORTED);
}

void RequestBuilder::validatePath()
{
    if (request_path.empty() || request_path[0] != '/')
        throw HTTPException(HTTPCode::BAD_REQUEST);
}

void RequestBuilder::validateHeader()
{
    HashMap<std::string, std::string>::iterator it = headers.find(_HEADER_HOST_);
    if (it == headers.end())
        throw HTTPException(HTTPCode::BAD_REQUEST);
}

void RequestBuilder::invalidPath()
{
    throw HTTPException(HTTPCode::BAD_REQUEST);
}

void RequestBuilder::check()
{
    validateMethod();
    validateProtocol();
    validatePath();
    validateHeader();
    is_validated = true;
}

Request RequestBuilder::build() const
{
    Method m = Method::UNKNOWN;
    for (size_t i = 0; i < map_size; ++i) 
    {
        if (method == map[i].str) 
        {
            m = map[i].val;
            break;
        }
    }
    return Request(m, request_path, query_path, protocol, content_length, headers, body);
}

void RequestBuilder::print() const 
{
    HashMap<std::string, std::string>::const_iterator it = headers.find(_HEADER_HOST_);
    if (it != headers.end())
    {
        std::cout << "--- REQUEST DEBUG ---" << std::endl;
        std::cout << "Method: [" << method << "]" << std::endl;
        std::cout << "Path:   [" << request_path << "]" << std::endl;
        std::cout << "Proto:  [" << protocol << "]" << std::endl;
        std::cout << "Query:  [" << query_path << "]" << std::endl;
        std::cout << "Host:  [" << it->second << "]" << std::endl;
        std::cout << "Content-Length: " << content_length << std::endl;
        std::cout << "Headers Count: " << headers.size() << std::endl;
        std::cout << "Body Size: " << body.size() << std::endl;
        if (!body.empty())
        std::cout << "Body (first 20 bytes): " << std::string(body.begin(), body.begin() + std::min(body.size(), (size_t)20)) << "..." << std::endl;
        std::cout << "Complete: " << (is_validated ? "YES" : "NO") << std::endl;
        std::cout << "---------------------" << std::endl;
    }
}

const bool &RequestBuilder::getCompleteStatus() const { return parsing_is_complete; }
const bool &RequestBuilder::isHeaderParsed() const { return header_is_parsed; }
const bool &RequestBuilder::isValidated() const { return is_validated; }
void RequestBuilder::setValidateStatus(int status) { is_validated = status; }

