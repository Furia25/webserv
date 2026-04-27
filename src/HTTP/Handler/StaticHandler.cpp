/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StaticHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 10:50:35 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/27 18:58:19 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/Handler/StaticHandler.hpp"
# include "HTTP/Response.hpp"
# include "HTTP/HttpTypes.hpp"
# include "Utils/FileSystem.hpp"
# include "StaticHandler.hpp"

void StaticHandler::onCreation()
{
	switch (this->request.getMethod())
	{
	case Method::GET:
		if (FileSystem::isDirectory(physicalPath) && !this->staticConfig.autoindex)
			throw HTTPException(HTTPCode::FORBIDDEN); /*NO BREAK NEEDED FALLBACK*/
	case Method::HEAD:
		if (!FileSystem::exists(physicalPath))
			throw HTTPException(HTTPCode::NOT_FOUND);
		if (!FileSystem::isReadable(physicalPath))
			throw HTTPException(HTTPCode::UNAUTHORIZED);
	default:
		throw HTTPException(HTTPCode::METHOD_NOT_ALLOWED);
	break;
	}
	std::string separator = (physicalPath.empty() || physicalPath[physicalPath.length() - 1] == '/') ? "" : "/";
	std::string index_file = physicalPath + separator + this->staticConfig.index;
	if (!staticConfig.index.empty() && FileSystem::exists(index_file))
		physicalPath = index_file;
}

StaticHandler::~StaticHandler() {}

void StaticHandler::onExecute()
{
	switch (state)
	{
	case INIT: state = SEND_HEADERS; break;
	case SEND_HEADERS:
		fileStream.open(physicalPath.c_str(), std::ios::binary);
		if (!fileStream.is_open()) 
			throw HTTPException(HTTPCode::INTERNAL_SERVER_ERROR);

		size_t	fileSize = FileSystem::getFileSize(physicalPath);
		MIME	mime_type = MIME::from_extension(FileSystem::getExtension(physicalPath));
		Response::buildFileHeaderResponse(connection, statusCode, mime_type, fileSize);
		
		if (request.getMethod() == Method::HEAD)
		{
			fileStream.close();
			state = FINISHED;
		}
		else
			state = SEND_BODY;
	break ;

	case SEND_BODY:
		char buffer[8192];
		fileStream.read(buffer, sizeof(buffer));
		std::streamsize bytes_read = fileStream.gcount();
		
		if (bytes_read > 0)
			Response::sendBodyChunk(connection, reinterpret_cast<const uint8_t*>(buffer), static_cast<size_t>(bytes_read));
		
		if (fileStream.eof() || fileStream.fail())
		{
			fileStream.close();
			state = FINISHED;
		}
	break;

	case FINISHED:
		this->setFinished();
		return ;
	}
}

