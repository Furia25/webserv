/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileWriter.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 13:49:30 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/29 19:35:04 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Utils/FileWriter.hpp"
# include "HTTP/HttpTypes.hpp"

void FileWriter::open(const std::string& path, bool append)
{
	this ->filePath = path;
	std::ios_base::open_mode mode = std::ios::binary | std::ios::out;
	if (append)
		mode |=std::ios::app;
	else
		mode |= std::ios::trunc;
	this->fileStream.open(filePath.c_str(), mode);
	if (!this->fileStream.is_open())
	{
		hasError = true;
		throw HTTPException(HTTPCode::INTERNAL_SERVER_ERROR);
	}
	this->bytesWrittenTotal = 0;
}

size_t FileWriter::writeChunk(const std::vector<uint8_t>& buffer)
{
	if (!this->fileStream.is_open() || buffer.empty() | hasError)
		return 0;
	this->fileStream.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
	if (this->fileStream.fail())
		return 0;
	this->bytesWrittenTotal += buffer.size();
	return buffer.size();
}

size_t FileWriter::writeChunk(const char* data, size_t size)
{
	if (!data | size == 0 | hasError)
		return 0;
	fileStream.write(data, size);
	if (fileStream.fail())
	{
		hasError = true;
		return 0;
	}
	bytesWrittenTotal += size;
	return size;
}

bool FileWriter::getError() const
{
	return this->hasError;
}

void FileWriter::close()
{
	if (this->fileStream.is_open())
		this->fileStream.close();
}


size_t FileWriter::getBytesWritten()const
{
	return this->bytesWrittenTotal;
}

void FileWriter::abort()
{
	this->close();
	std::remove(filePath.c_str());
}