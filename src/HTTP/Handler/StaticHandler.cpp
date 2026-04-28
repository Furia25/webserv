/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StaticHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 10:50:35 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/28 14:25:48 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/Handler/StaticHandler.hpp"
# include "HTTP/Response.hpp"
# include "HTTP/HttpTypes.hpp"
# include "Utils/FileSystem.hpp"
# include "HTTP/Handler/StaticHandler.hpp"
# include <dirent.h>

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

void	StaticHandler::handleDelete()
{
	if (!FileSystem::exists(physicalPath))
		throw HTTPException(HTTPCode::NOT_FOUND);
	else if (FileSystem::isDirectory(physicalPath))
		throw HTTPException(HTTPCode::FORBIDDEN);
	else if (!FileSystem::isWritable(physicalPath))
		throw HTTPException(HTTPCode::FORBIDDEN);
	else
	{
		if (std::remove(physicalPath.c_str()) == 0)
			Response::buildEmptyResponse(connection, HTTPCode::NO_CONTENT);
		else
		{
			throw HTTPException(HTTPCode::INTERNAL_SERVER_ERROR);
		}
	}
	this->setFinished();
}

void StaticHandler::handleAutoindex()
{
    std::string path = this->request.getPath();
    std::string body = "<html><head><title>Index of " + path + "</title></head>";
    body += "<body><h1>Index of " + path + "</h1><hr><ul>";

    DIR *dir = opendir(this->physicalPath.c_str());
    if (dir)
    {
        struct dirent *ent;
        while ((ent = readdir(dir)) != NULL)
        {
            std::string name = ent->d_name;
            if (name == ".")
                continue;

            std::string sep = (path.empty() || path[path.length() - 1] == '/') ? "" : "/";
            std::string link = path + sep + name;
            body += "<li><a href=\"" + link + "\">" + name + "</a></li>";
        }
        closedir(dir);
        body += "</ul><hr></body></html>";

        Response::buildRawResponse(this->connection, HTTPCode::OK, MIME::html, body);
    }
    else
    {
        throw HTTPException(HTTPCode::FORBIDDEN);
    }
}

void StaticHandler::onExecute()
{
	switch (state)
	{
	case INIT:
	{
		if (this->request.getMethod() == Method::DELETE)
		{
			this->handleDelete();
			state = FINISHED;
			break;
		}
		if (FileSystem::isDirectory(this->physicalPath) && this->staticConfig.autoindex)
		{
			state = SEND_AUTOINDEX;
			break;
		}
		state = SEND_HEADERS; 
	}
	break;
	
	case SEND_AUTOINDEX:
	{
		this->handleAutoindex();
		state = FINISHED;
	}
	break;

	case SEND_HEADERS:
	{
		this->fileReader.open(physicalPath);

		size_t	fileSize = this->fileReader.getFileSize();
		MIME	mime_type = MIME::from_extension(FileSystem::getExtension(physicalPath));
		
		Response::buildFileHeaderResponse(connection, statusCode, mime_type, fileSize);
		
		if (request.getMethod() == Method::HEAD)
		{
			this->fileReader.close();
			state = FINISHED;
		}
		else
			state = SEND_BODY;
	}
	break ;

	case SEND_BODY:
	{
		std::vector<uint8_t> buffer;
		size_t bytes_read = this->fileReader.readChunk(buffer, 8192);
		
		if (bytes_read > 0)
			Response::sendBodyChunk(connection, buffer.data(), bytes_read);
		
		if (this->fileReader.hasFinished())
		{
			this->fileReader.close();
			state = FINISHED;
		}
	}
	break;

	case FINISHED:
		this->setFinished();
		return ;
	}
}

