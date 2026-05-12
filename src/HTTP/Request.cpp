/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 15:03:13 by antoine           #+#    #+#             */
/*   Updated: 2026/05/12 16:47:52 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/Request.hpp"

#define _IS_ONE_MO_ 1048576

Request::Request() : method(Method::GET),
		path("/"),
		query_string(""),
		protocol(""),
		content_length(0),
		is_chunk_encoding(false)
{}

void	Request::initBody(const std::string& path, bool stream)
{
	body.init(this->content_length, path, stream);
}

void	Request::feedBody(const uint8_t* data, size_t size)
{
	body.feed(data, size);
}

bool	Request::isBodyComplete() const
{
	return this->body.isComplete();
}

bool	Request::checkBodyOverflow() const
{
	return this->body.checkOverflow();
}

void	Request::finishBody()
{
	return this->body.finish();
}

const Body&	Request::getBody() const
{
	return this->body;
}

Body&	Request::getBody()
{
	return this->body;
}

size_t	Request::getBodySize() const
{
	return this->body.getReceivedSize();
}

void Request::setCookies(const Cookies& cookies) { this->cookies = cookies; }
void Request::setHeaders(const Headers& headers) { this->headers = headers; }

const Request::Headers&	Request::getHeaders() const 
{
	return headers;
}

Request::Headers&	Request::getHeaders() 
{
	return headers;
}

const Request::Cookies&	Request::getCookies() const { return cookies; }
Request::Cookies&	Request::getCookies() { return cookies; }

bool	Request::isChunked() const 
{
	return this->is_chunk_encoding;
}

size_t	Request::isLessThanOneMO() const
{
	return (this->content_length < _IS_ONE_MO_);
}

Body::Body() : fileWriter(NULL), isStreaming(false), expectedSize(0), receivedSize(0), isFinished(false) {}

void	Body::init(size_t expected, const std::string& path, bool stream)
{
	this->expectedSize = expected;
	this->isStreaming = stream;
	this->destinationPath = path;
	this->isFinished = false;

	if (this->isStreaming)
	{
		this->fileWriter = new FileWriter();
		this->fileWriter->open(this->destinationPath);
	}
	else
		memoryBuffer.reserve(expectedSize);
}

void    Body::feed(const uint8_t* data, size_t size)
{
	if (size == 0)
		return ;
	
	if (this->isStreaming)
		fileWriter->writeChunk(data, size);
	else
		memoryBuffer.insert(memoryBuffer.end(), data, data + size);
	receivedSize += size;
}

void	Body::setIsFinished(bool status)
{
	this->isFinished = status;
}

bool	Body::isComplete() const 
{
	if (this->isStreaming && this->expectedSize == 0)
		return this->isFinished;
	
	return this->receivedSize >= this->expectedSize;
}

bool	Body::checkOverflow() const
{
	return this->receivedSize > this->expectedSize;
}

void	Body::finish()
{
	if (this->fileWriter)
		this->fileWriter->close();
	this->isFinished = true;
}

void	Body::reset() 
{
	if (this->fileWriter) 
	{
		if (this->isStreaming)
		{
			this->fileWriter->close();
			delete this->fileWriter;
		}
		this->fileWriter = NULL;
	}
}

bool	Body::getIsStreaming() const
{
	return this->isStreaming;
}


const	std::vector<uint8_t>&	Body::getMemoryBuffer() const
{
	return this->memoryBuffer;
}

const std::string&	Body::getFilePath() const
{
	return this->destinationPath;
}

size_t	Body::getReceivedSize() const
{
	return this->receivedSize;
}

void	Body::setFilePath(const std::string& path)
{
	this->destinationPath = path;
}

void	Body::setIsStreaming(bool stream)
{
	this->isStreaming = stream;
}

FileWriter*	Body::getFileWriter()const 
{
	return this->fileWriter;
}

Body::~Body()
{
	if (isStreaming && !destinationPath.empty())
	{
		if (fileWriter && !isFinished)
		{
			if (fileWriter->isOpen())
				fileWriter->close();
			std::remove(this->destinationPath.c_str());
		}
	}
	this->reset();
}
