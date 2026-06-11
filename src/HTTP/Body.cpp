/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Body.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/12 17:06:01 by antoine           #+#    #+#             */
/*   Updated: 2026/06/11 16:50:33 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/Body.hpp"
# include "Utils/FileSystem.hpp"
# include "HTTP/HTTPTypes.hpp"
# include "Logger.hpp"

#define _IS_ONE_MO_ 1048576

Body::Body() : fileWriter(NULL), expectedSize(0), receivedSize(0), destinationPath(""), isFinished(false), chunkState(CHUNK_SIZE), neededBytes(0), maxBodySize(0)
{
}

void	Body::init(size_t expected, const std::string& path, size_t maxBody)
{
	this->expectedSize = expected;
	this->destinationPath = path;
	this->isFinished = false;
	this->maxBodySize = maxBody;

	std::string destination_temp = this->destinationPath;
	this->fileWriter = new FileWriter();
	this->fileWriter->open(destination_temp);
}

void    Body::feed(const uint8_t* data, size_t size)
{
	if (size == 0)
		return ;
	fileWriter->writeChunk(data, size);
	receivedSize += size;
}

void	Body::setIsFinished(bool status)
{
	this->isFinished = status;
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
		this->fileWriter->close();
		delete this->fileWriter;
		this->fileWriter = NULL;
		std::remove(this->destinationPath.c_str());
	}
	this->fileWriter = NULL;
	this->expectedSize = 0;
	this->receivedSize = 0;
	this->destinationPath = "";
	this->isFinished = false;
	this->chunkState = CHUNK_SIZE;
	this->neededBytes = 0;
	this->sizeBuffer.clear();
}

void	Body::handleChunkSize(const uint8_t* fragment, size_t& i)
{
	char c = fragment[i++];
	if (c == '\r') 
		return;
	if (c == '\n') 
	{
		this->neededBytes = std::strtoul(this->sizeBuffer.c_str(), NULL, 16);
		if (this->maxBodySize > 0 && this->neededBytes > this->maxBodySize)
			throw HTTPException(HTTPCode::PAYLOAD_TOO_LARGE);
		this->sizeBuffer.clear();
		this->chunkState = (this->neededBytes == 0) ? CHUNK_COMPLETE : CHUNK_DATA;
	}
	else
		this->sizeBuffer += c;
}

void	Body::handleChunkData(const uint8_t* fragment, size_t& i, size_t size)
{
	size_t remainingInFragment = size - i;
	size_t toWrite = (remainingInFragment < this->neededBytes) ? remainingInFragment : this->neededBytes;

	if (toWrite > 0) 
	{
		this->feed(fragment + i, toWrite); 
		i += toWrite;
		this->neededBytes -= toWrite;
	}
	if (this->neededBytes == 0)
		this->chunkState = CHUNK_TRAILER;
}

void	Body::handleChunkTrailer(const uint8_t* fragment, size_t& i)
{
	char c = fragment[i++];
	if (c == '\n')
		this->chunkState = CHUNK_SIZE;
}

bool	Body::hasFinished() const
{
	return this->isFinished;
}

size_t	Body::feedChunked(const uint8_t* fragment, size_t size)
{
	size_t i = 0;
	while (i < size && !this->hasFinished())
	{
		switch (this->chunkState)
		{
			case CHUNK_SIZE:
			{
				this->handleChunkSize(fragment, i);
				break;
			}
			case CHUNK_DATA:
			{
				this->handleChunkData(fragment, i, size);
				break;
			}
			case CHUNK_TRAILER:
			{
				this->handleChunkTrailer(fragment, i);
				break;
			}
			case CHUNK_COMPLETE:
			{
				this->setIsFinished(true);
				i++;
				return i;
			}
		}
	}
	return i;
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

FileWriter*	Body::getFileWriter()const 
{
	return this->fileWriter;
}

Body::~Body() 
{
	if (this->fileWriter) 
	{
		if (FileSystem::exists(this->destinationPath)) 
			std::remove(this->destinationPath.c_str());
		delete this->fileWriter;
		this->fileWriter = NULL;
	}
}
