/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StaticHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 10:50:35 by antbonin          #+#    #+#             */
/*   Updated: 2026/06/09 21:25:10 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/Response.hpp"
# include "HTTP/HTTPTypes.hpp"
# include "Utils/FileSystem.hpp"
# include "HTTP/Handler/StaticHandler.hpp"
# include <dirent.h>

void StaticHandler::onCreation()
{
	if (FileSystem::isDirectory(this->physicalPath))
	{
		const std::string separator = (physicalPath.empty() || physicalPath[physicalPath.length() - 1] == '/') ? "" : "/";
		std::string index_file = this->physicalPath + separator + this->staticConfig.index;
		if (!this->staticConfig.index.empty() && FileSystem::exists(index_file))
			this->physicalPath = index_file;
	}
	switch (this->request.method)
	{
		case Method::GET:
		case Method::HEAD:
		{
			if (FileSystem::isDirectory(physicalPath))
			{
				if (!this->staticConfig.autoindex)
					throw HTTPException(HTTPCode::NOT_FOUND);
			}
			else 
			{
				if (!FileSystem::exists(physicalPath))
					throw HTTPException(HTTPCode::NOT_FOUND);
				if (!FileSystem::isReadable(physicalPath))
					throw HTTPException(HTTPCode::FORBIDDEN);
			}
		}
			break;
		case Method::DELETE:
			if (!FileSystem::exists(physicalPath))
				throw HTTPException(HTTPCode::NOT_FOUND);
			break;
		default:
		throw HTTPException(HTTPCode::METHOD_NOT_ALLOWED);
	}
}

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
			Response(this->connection, HTTPCode::NO_CONTENT).sendDefaults(this->request, &this->staticConfig).sendEnd();
		else
			throw HTTPException(HTTPCode::INTERNAL_SERVER_ERROR);
	}
	this->setFinished();
}

void StaticHandler::handleAutoindex()
{
	std::string	path = this->request.path;
	DIR *dir = opendir(this->physicalPath.c_str());
	if (!dir)
		throw HTTPException(HTTPCode::FORBIDDEN);

	try
	{
		response.sendStatusLine(HTTPCode::OK)
			.sendContentType(MIME::html)
			.sendDefaults(this->request, this->routeResult.route)
			.setChunked();

		if (this->request.method == Method::HEAD)
		{
			closedir(dir);
			response.sendEnd();
			this->setFinished();
			return;
		}

		std::string	headerHtml = "<html><head><title>Index of " + path + "</title></head><body><h1>Index of " + path + "</h1><hr><ul>";
		response.sendChunk(headerHtml);

		struct dirent *ent;
		while ((ent = readdir(dir)) != NULL)
		{
			std::string name = ent->d_name;
			if (name == ".")
				continue;

			std::string sep = (path.empty() || path[path.length() - 1] == '/') ? "" : "/";
			std::string body = "<li><a href=\"" + path + sep + name + "\">" + name + "</a></li>";

			response.sendChunk(body);
		}
		closedir(dir);
		response.sendChunk("</ul><hr></body></html>");
		response.sendEnd();
	}
	catch (...) { closedir(dir); throw ;}

	this->setFinished();
}

void StaticHandler::onExecute()
{
	switch (state)
	{
	case INIT:
	{
		if (this->request.method == Method::DELETE)
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

		size_t		fileSize = this->fileReader.getFileSize();
		std::string	extension = FileSystem::getExtension(physicalPath);
		MIME		mime_type = MIME::from_extension(extension);
		response.sendStatusLine(statusCode)
			.sendDefaults(this->request, this->routeResult.route)
			.sendContentType(mime_type)
			.sendContentLength(fileSize);

		if (request.method == Method::HEAD)
		{
			this->fileReader.close();
			state = FINISHED;
		}
		else
			state = SEND_BODY;
	}
	break;
	
	case SEND_BODY:
	{
		char buffer[8192];
		size_t bytes_read = this->fileReader.readChunk(buffer, 8192);

		if (bytes_read > 0)
			response.sendBody((uint8_t *)buffer, bytes_read);

		if (this->fileReader.hasFinished())
		{
			this->fileReader.close();
			this->state = FINISHED;
		}
		break;
	}

	case FINISHED:
		response.sendEnd();	
		this->setFinished();
		return ;
	}
}

