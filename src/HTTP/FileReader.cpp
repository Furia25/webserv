/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileReader.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 12:27:28 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/28 13:06:39 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Utils/FileReader.hpp"
# include "Utils/FileSystem.hpp"
# include "HTTP/HttpTypes.hpp"

FileReader::FileReader() : fileSize(0), bytesReadTotal(0), isEOF(false)
{
}

FileReader::~FileReader()
{
	this->close();
}

void FileReader::open(const std::string& path)
{
	if (!FileSystem::exists(path) || !FileSystem::isFile(path) || !FileSystem::isReadable(path))
		throw HTTPException(HTTPCode::NOT_FOUND);

	this->filePath = path;
	this->fileStream.open(path.c_str(), std::ios::binary | std::ios::ate);
	
	if (!this->fileStream.is_open())
		throw HTTPException(HTTPCode::INTERNAL_SERVER_ERROR);

	this->fileSize = this->fileStream.tellg();
	this->fileStream.seekg(0, std::ios::beg);
	
	this->bytesReadTotal = 0;
	this->isEOF = false;
}

size_t FileReader::readChunk(std::vector<uint8_t>& buffer, size_t chunkSize)
{
	if (!this->fileStream.is_open() || this->isEOF)
		return 0;
	
	uint8_t buffer_tmp[chunkSize];
	this->fileStream.read((char *)buffer_tmp, chunkSize);

	std::streamsize bytesRead = this->fileStream.gcount();

	if (bytesRead > 0)
	{
		buffer.insert(buffer.end(), buffer_tmp, buffer_tmp + bytesRead);
		this->bytesReadTotal += bytesRead;
	}
	
	if (this->fileStream.eof() || bytesRead == 0)
		this->isEOF = true;
	return bytesRead;
}

void FileReader::close()
{
	fileStream.close();
}
bool FileReader::hasFinished() const
{
	return this->isEOF;
}

size_t FileReader::getFileSize()const
{
	return this->fileSize;
}

