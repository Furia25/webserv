/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorHandler.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 14:39:49 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/29 17:35:56 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/Handler/ErrorHandler.hpp"
# include "Utils/FileSystem.hpp"
# include <string>

void ErrorHandler::onExecute()
{
	switch (this->state)
	{
	case INIT:
	{
		if (hostConfig->error_fallbacks.contain(statusCode))
		{
			std::string errorFile = hostConfig->error_fallbacks.at(statusCode);
			physicalPath = hostConfig->root + "/" + errorFile;
		}

		if (!physicalPath.empty() && FileSystem::exists(physicalPath) && FileSystem::isFile(physicalPath))
			this->state = SEND_HEADERS;
		else
			this->state = SEND_DEFAULT_ERROR;
		break;
	}

	case SEND_HEADERS:
	{
		try 
		{
			this->fileReader.open(physicalPath);
			size_t fileSize = this->fileReader.getFileSize();
			MIME mime_type = MIME::from_extension(FileSystem::getExtension(physicalPath));

			Response::buildFileHeaderResponse(connection, statusCode, mime_type, fileSize);
			this->state = SEND_BODY;
		}
		catch (...)
		{
			this->state = SEND_DEFAULT_ERROR;
		}
		break;
	}

	case SEND_BODY:
	{
		std::vector<uint8_t> buffer;
		size_t bytes_read = this->fileReader.readChunk(buffer, 8192);

		if (bytes_read > 0)
			Response::sendBodyChunk(connection, buffer.data(), bytes_read);

		if (this->fileReader.hasFinished())
		{
			this->fileReader.close();
			this->state = FINISHED;
		}
		break;
	}

	case SEND_DEFAULT_ERROR:
	{
		std::stringstream ss;
		ss << "<html><head><title>Error " << static_cast<int>(statusCode) << "</title></head>" 
		<< "<body><center><h1>" << static_cast<int>(statusCode) << "</h1><hr>"
		<< "WebServ/1.0 (Custom Error Page)</center></body></html>";
        
		std::string body = ss.str();
		Response::buildRawResponse(connection, statusCode, MIME::html, body);
		this->state = FINISHED;
		break;
	}

	case FINISHED:
		this->setFinished();
		break;
	}
}
