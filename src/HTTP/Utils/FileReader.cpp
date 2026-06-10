/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileReader.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 12:27:28 by antbonin          #+#    #+#             */
/*   Updated: 2026/06/10 17:20:47 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/Utils/FileReader.hpp"
# include "Utils/FileSystem.hpp"
# include "HTTP/HTTPTypes.hpp"

FileReader::FileReader() : fileSize(0), bytesReadTotal(0), isEOF(false) {}

FileReader::~FileReader()
{
	this->close();
}

void FileReader::open(const std::string& path)
{
	if (this->fileStream.is_open())
		this->fileStream.close();

	this->fileStream.clear();

	if (!FileSystem::exists(path) || !FileSystem::isFile(path) || !FileSystem::isReadable(path))
		throw HTTPException(HTTPCode::NOT_FOUND);

	this->filePath = path;

	struct stat stat_buf;
	if (stat(path.c_str(), &stat_buf) != 0)
		throw HTTPException(HTTPCode::INTERNAL_SERVER_ERROR);

	this->fileSize = stat_buf.st_size;
	this->fileStream.open(path.c_str(), std::ios::in | std::ios::binary);

	if (!this->fileStream.is_open())
		throw HTTPException(HTTPCode::INTERNAL_SERVER_ERROR);

	this->bytesReadTotal = 0;
	this->isEOF = false;
}

size_t FileReader::readChunk(char *buffer, size_t chunkSize)
{
	if (!this->fileStream.is_open() || this->isEOF)
		return 0;

	this->fileStream.read(buffer, chunkSize);
	std::streamsize bytesRead = this->fileStream.gcount();

	this->bytesReadTotal += bytesRead;

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

