/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AHandler.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 15:57:58 by vdurand           #+#    #+#             */
/*   Updated: 2026/05/06 17:30:32 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/AHandler.hpp"
# include "Utils/FileSystem.hpp"
# include "HTTP/Response.hpp"

bool AHandler::execute()
{
	if (this->finished)
		return false;

	try
	{
		this->onExecute();
	}
	catch (const HTTPException& http_exception)
	{
		this->statusCode = http_exception.getStatusCode();
		this->initError();
	}
	catch (const std::exception& e)
	{
		this->statusCode = HTTPCode::INTERNAL_SERVER_ERROR;
		this->initError();
	}

	return !this->finished;
}

/*void AHandler::handleError()
{
	if (!this->physicalPath.empty() && FileSystem::isFile(this->physicalPath))
	{
		if (!this->fileHeaderSent)
		{
			this->fileReader.open(this->physicalPath);
			MIME	mime_type = MIME::from_extension(FileSystem::getExtension(physicalPath));
			Response::buildFileHeaderResponse(this->connection, this->statusCode, mime_type, this->fileReader.getFileSize());
			this->fileHeaderSent = true;
		}
		if (!this->fileReader.hasFinished())
		{
			std::vector<uint8_t> buffer;
			size_t bytesRead = this->fileReader.readChunk(buffer, 8192);
			if (bytesRead > 0)
				Response::sendBodyChunk(this->connection, buffer.data(), bytesRead);
		}
		if (this->fileReader.hasFinished())
		{
			this->fileReader.close();
			this->setFinished();
		}
		else
		{
			Response::buildErrorResponse(this->connection, this->statusCode);
			this->setFinished();
		}
	}
}*/

void AHandler::initError()
{
	HashMap<HTTPCode, std::string>::const_iterator it = hostConfig->error_fallbacks.find(this->statusCode);
	if (it != hostConfig->error_fallbacks.end())
	{
		std::string error_path = it->second;
		if (error_path.length() > 0 && error_path[0] != '/')
			error_path = hostConfig->root + "/" + error_path;
		else
			error_path = hostConfig->root + error_path;

		if (FileSystem::exists(error_path) && FileSystem::isFile(error_path) && FileSystem::isReadable(error_path))
		{
			this->physicalPath = error_path;
			return;
		}
	}
	Response::buildErrorResponse(connection, this->statusCode);
}