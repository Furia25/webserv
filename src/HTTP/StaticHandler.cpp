/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StaticHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 10:50:35 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/27 12:56:03 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/StaticHandler.hpp"
# include "HTTP/Response.hpp"
# include "HTTP/HttpTypes.hpp"
# include "Utils/FileSystem.hpp"

StaticHandler::StaticHandler(const Request &req, Connection &connection, const std::string& physical_path, const Config::ServerConfig* host_ptr, HTTPCode statusCode)
	: request(req), connection(connection), physical_path(physical_path), host(host_ptr), isFinished(false), state(INIT), statusCode(statusCode)
{
}

StaticHandler::~StaticHandler()
{
	if (file_stream.is_open())
		file_stream.close();
}

static std::string getMimeType(const std::string& ext)
{
	if (ext == "html" || ext == "htm") return "text/html";
	if (ext == "css") return "text/css";
	if (ext == "js") return "text/javascript";
	if (ext == "json") return "application/json";
	if (ext == "xml") return "application/xml";
	if (ext == "png") return "image/png";
	if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
	if (ext == "gif") return "image/gif";
	if (ext == "webp") return "image/webp";
	if (ext == "svg") return "image/svg+xml";
	if (ext == "ico") return "image/x-icon";
	if (ext == "txt" || ext == "md" || ext == "csv") return "text/plain";
	if (ext == "mp4") return "video/mp4";
	if (ext == "webm") return "video/webm";
	if (ext == "pdf") return "application/pdf";
	if (ext == "zip") return "application/zip";
	
	return "application/octet-stream"; 
}

void StaticHandler::handleError(HTTPCode code)
{
	HashMap<HTTPCode, std::string>::const_iterator it = host->error_fallbacks.find(code);
	if (it != host->error_fallbacks.end())
	{
		std::string error_path = it->second;
		if (error_path.length() > 0 && error_path[0] != '/')
			error_path = host->root + "/" + error_path;
		else
			error_path = host->root + error_path;

		if (FileSystem::exists(error_path) && FileSystem::isFile(error_path) && FileSystem::isReadable(error_path))
		{
			this->physical_path = error_path;
			this->statusCode = code;
			this->state = SEND_HEADERS;
			return;
		}
	}
	Response::buildErrorResponse(connection, code);
	this->state = FINISHED;
}

bool StaticHandler::execute()
{
	if (state == INIT)
	{
		state = SEND_HEADERS;
		return false;
	}

	if (state == SEND_HEADERS)
	{
		file_stream.open(physical_path.c_str(), std::ios::binary);
		if (!file_stream.is_open()) 
		{
			handleError(HTTPCode::INTERNAL_SERVER_ERROR);
			return false;
		}

		size_t fileSize = FileSystem::getFileSize(physical_path);
		std::string ext = FileSystem::getExtension(physical_path);
		ext = getMimeType(ext);
		Response::buildFileHeaderResponse(connection, statusCode, ext, fileSize);
		
		if (request.getMethod() == Method::HEAD)
		{
			file_stream.close();
			state = FINISHED;
		}
		else
			state = SEND_BODY;
		return false; 
	}

	if (state == SEND_BODY)
	{
		char buffer[8192];
		file_stream.read(buffer, sizeof(buffer));
		std::streamsize bytes_read = file_stream.gcount();
		
		if (bytes_read > 0)
			Response::sendBodyChunk(connection, reinterpret_cast<const uint8_t*>(buffer), static_cast<size_t>(bytes_read));
		
		if (file_stream.eof() || file_stream.fail())
		{
			file_stream.close();
			state = FINISHED;
		}
		return false;
	}

	if (state == FINISHED)
	{
		isFinished = true;
		return true;
	}
	return true;
}
