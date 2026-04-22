/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Handler.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 18:00:00 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/22 14:03:55 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/Handler.hpp"
# include "HTTP/Response.hpp"
# include "HTTP/HttpTypes.hpp"
# include "Utils/FileSystem.hpp"
# include <dirent.h>

StaticHandler::StaticHandler(const Request &req, const RouteConfig *route, Connection &connection, const std::string& physical_path)
    : request(req), route(route), connection(connection), physical_path(physical_path), isFinished(false), state(INIT)
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

bool StaticHandler::execute()
{
    if (state == INIT)
    {
        if (!FileSystem::exists(physical_path))
        {
            Response::buildErrorResponse(connection, HTTPCode::NOT_FOUND);
            state = FINISHED;
            return false;
        }

        if (FileSystem::isDirectory(physical_path))
        {
            if (request.getMethod() == Method::GET)
            {
                const StaticConfig* static_config = static_cast<const StaticConfig*>(route);
                std::string index_file = physical_path + (physical_path.empty() || physical_path[physical_path.length() - 1] == '/' ? "" : "/") + static_config->index;
                if (!static_config->index.empty() && FileSystem::exists(index_file))
                    physical_path = index_file;
                else if (static_config->autoindex)
                {
                    std::string autoindexBody = "<html><head><title>Index of " + request.getPath() + "</title></head><body><h1>Index of " + request.getPath() + "</h1><hr><ul>";
                    DIR* dir = opendir(physical_path.c_str());
                    if (dir)
                    {
                        struct dirent* ent;
                        while ((ent = readdir(dir)) != NULL)
                        {
                            std::string name = ent->d_name;
                            std::string link = request.getPath() + (request.getPath().empty() || request.getPath()[request.getPath().length() - 1] == '/' ? "" : "/") + name;
                            autoindexBody += "<li><a href=\"" + link + "\">" + name + "</a></li>";
                        }
                        closedir(dir);
                        autoindexBody += "</ul><hr></body></html>";
                        Response::buildRawResponse(connection, HTTPCode::OK, "text/html", autoindexBody);
                    }
                    else
                        Response::buildErrorResponse(connection, HTTPCode::FORBIDDEN);
                    state = FINISHED;
                    return false;
                }
                else
                {
                    Response::buildErrorResponse(connection, HTTPCode::FORBIDDEN);
                    state = FINISHED;
                    return false;
                }
            }
            else if (request.getMethod() == Method::DELETE)
            {
                Response::buildErrorResponse(connection, HTTPCode::FORBIDDEN);
                state = FINISHED;
                return false;
            }
        }

        if (request.getMethod() == Method::GET)
        {
            if (!FileSystem::isReadable(physical_path))
            {
                Response::buildErrorResponse(connection, HTTPCode::FORBIDDEN);
                state = FINISHED;
                return false;
            }
            state = SEND_HEADERS;
        }
        else if (request.getMethod() == Method::DELETE)
        {
            if (!FileSystem::isWritable(physical_path))
            {
                Response::buildErrorResponse(connection, HTTPCode::FORBIDDEN);
                state = FINISHED;
                return false;
            }

            if (std::remove(physical_path.c_str()) == 0)
                Response::buildEmptyResponse(connection, HTTPCode::NO_CONTENT);
            else
                Response::buildErrorResponse(connection, HTTPCode::INTERNAL_SERVER_ERROR);
            state = FINISHED;
        }
        else
        {
            Response::buildErrorResponse(connection, HTTPCode::METHOD_NOT_ALLOWED);
            state = FINISHED;
        }
        return false;
    }

    if (state == SEND_HEADERS)
    {
        file_stream.open(physical_path.c_str(), std::ios::binary);
        if (!file_stream.is_open()) 
        {
            Response::buildErrorResponse(connection, HTTPCode::INTERNAL_SERVER_ERROR);
            state = FINISHED;
            return false;
        }

        size_t fileSize = FileSystem::getFileSize(physical_path);
        std::string ext = FileSystem::getExtension(physical_path);
        ext = getMimeType(ext);
        Response::buildFileHeaderResponse(connection, HTTPCode::OK, ext, fileSize);
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

