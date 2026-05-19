/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Body.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/12 17:06:01 by antoine           #+#    #+#             */
/*   Updated: 2026/05/19 15:48:47 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/Body.hpp"
# include "Utils/FileSystem.hpp"
# include "HTTP/HTTPTypes.hpp"
# include "Logger.hpp"

#define _IS_ONE_MO_ 1048576

Body::Body() : fileWriter(NULL), isStreaming(false), expectedSize(0), receivedSize(0), destinationPath(""), isFinished(false)
{
}

Body::Body(const Body& other) 
{
	this->isStreaming = other.isStreaming;
	this->isFinished = other.isFinished;
	this->destinationPath = other.destinationPath;
	this->fileWriter = other.fileWriter;
	const_cast<Body&>(other).fileWriter = NULL; 
}

Body&	Body::operator=(const Body& other) 
{
	if (this != &other) 
	{
		if (this->fileWriter) 
		{
			delete this->fileWriter;
			this->fileWriter = NULL;
		}
		this->isStreaming = other.isStreaming;
		this->isFinished = other.isFinished;
		this->destinationPath = other.destinationPath;
		this->fileWriter = other.fileWriter;
		const_cast<Body&>(other).fileWriter = NULL; 
	}
	return *this;
}

void ensureDirectoryExists(const std::string& filepath)
{
	size_t	lastSlash = filepath.find_last_of('/');
	if (lastSlash == std::string::npos)
		return;
	std::string dirPath = filepath.substr(0, lastSlash);
	if (dirPath.empty())
		return;
	struct stat st;
	if (stat(dirPath.c_str(), &st) != 0)
	{
		if (mkdir(dirPath.c_str(), 0755) != 0 && errno != EEXIST)
			throw HTTPCode(HTTPCode::INTERNAL_SERVER_ERROR);
	}
}

void	Body::init(size_t expected, const std::string& path, bool stream)
{
	this->expectedSize = expected;
	this->isStreaming = stream;
	this->destinationPath = path;
	this->isFinished = false;

	if (this->isStreaming)
	{
		std::string destination_temp = this->destinationPath + ".tmp";
		this->fileWriter = new FileWriter();
		this->fileWriter->open(destination_temp);
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

bool    Body::isLessThanOneMO()const
{
	return this->expectedSize < _IS_ONE_MO_;
}

size_t  Body::getSize()const 
{
	return this->receivedSize;
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
	if (this->fileWriter) 
	{
		if (isStreaming && FileSystem::exists(this->destinationPath)) 
        std::remove(this->destinationPath.c_str());
		delete this->fileWriter;
		this->fileWriter = NULL;
	}
}
