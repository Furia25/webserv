/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UploadHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/05 16:25:18 by antbonin          #+#    #+#             */
/*   Updated: 2026/06/08 20:56:04 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/Handler/UploadHandler.hpp"
# include "HTTP/HTTPHandler.hpp"
# include "HTTP/HTTPTypes.hpp"
# include "Utils/IntegerUtils.hpp"
# include "Utils/FileSystem.hpp"
# include <stdio.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/stat.h>

void	UploadHandler::onCreation()
{
	if (this->request.content_length == 0 && !this->request.is_chunked)
	{
		Logger::ERROR() << "Upload Failed 0 bytes sent";
		if (this->body.getFileWriter())
		{
			std::string path = this->body.getFileWriter()->getFilePath();
			this->body.getFileWriter()->close();
			if (!path.empty())
				FileSystem::removeFile(path);
		}
		throw HTTPException(HTTPCode::BAD_REQUEST);
	}
	if (this->uploadConfig.max_body_size > 0 && this->request.content_length > this->uploadConfig.max_body_size)
	{
		Logger::ERROR() << "Upload Payload too large in headers";
		std::remove(body.getFilePath().c_str());
		throw HTTPException(HTTPCode::PAYLOAD_TOO_LARGE);
	}
}

bool	isExtensionAllowed(const std::string& filename, const std::string& contentType, const std::vector<MIME>& allowed_exts) 
{
	if (allowed_exts.empty())
		return true;
	bool is_ok = false;
	if (!contentType.empty())
	{
		std::string clean_type = contentType;
		size_t semi_colon = clean_type.find(";");
		if (semi_colon != std::string::npos)
			clean_type = clean_type.substr(0, semi_colon);
		MIME content_mime = MIME::from_extension(clean_type);
		for (size_t i = 0; i < allowed_exts.size(); ++i)
		{
			if (content_mime == allowed_exts[i])
				is_ok = true;
		}
		if (!is_ok)
			return false;
	}
	size_t dotPos = filename.find_last_of('.');
	if (dotPos == std::string::npos)
		return false;
	std::string ext = filename.substr(dotPos + 1);
	MIME file_mime = MIME::from_extension(ext);
	for (size_t i = 0; i < allowed_exts.size(); ++i) 
	{
		if (file_mime == allowed_exts[i])
			is_ok = true;
	}
	return is_ok;
}

void	UploadHandler::onExecute()
{
	switch (this->copyState)
	{
		case INIT:
		{
			std::string current_filepath = body.getFilePath();
			std::string final_filepath = uploadConfig.alias + current_filepath.substr(4 , current_filepath.size() - 4);
			std::string content_type_val;
			std::cout << "final path : " << final_filepath << std::endl;
			HashMap<std::string, std::string>::const_iterator it = this->request.getHeaders().find("content-type");
			if (it != this->request.getHeaders().end())
			content_type_val = it->second;
			if (!isExtensionAllowed(final_filepath, content_type_val ,this->uploadConfig.allowed_extensions)) 
			{
				std::remove(current_filepath.c_str());
				throw HTTPException(HTTPCode::UNSUPPORTED_MEDIA);
			}
			if (FileSystem::exists(final_filepath) && !this->uploadConfig.allow_overwrite)
			{
				std::remove(current_filepath.c_str());
				throw HTTPException(HTTPCode::CONFLICT);
			}
			std::cout << "[DEBUG] current_filepath avant stat: [" << current_filepath << "]" << std::endl;
			std::cout << "[DEBUG] Le fichier existe-t-il ? " << FileSystem::exists(current_filepath) << std::endl;
			struct stat st;
			if (stat(current_filepath.c_str(), &st) != 0) 
			{
				std::cout << "coucouu\n";
				std::remove(current_filepath.c_str());
				throw HTTPException(HTTPCode::INTERNAL_SERVER_ERROR);
			}
			if (this->uploadConfig.max_body_size > 0 && static_cast<size_t>(st.st_size) > this->uploadConfig.max_body_size)
			{
				Logger::ERROR() << "Upload de-chunked too big for config : " << st.st_size;
				std::remove(current_filepath.c_str());
				throw HTTPException(HTTPCode::PAYLOAD_TOO_LARGE);
			}
			if (std::rename(current_filepath.c_str(), final_filepath.c_str()) != 0) 
			{
				std::remove(current_filepath.c_str());
				throw HTTPException(HTTPCode::INTERNAL_SERVER_ERROR);
			}
			response
			.sendStatusLine(HTTPCode::CREATED)
			.sendDefaults(this->request, this->routeResult.route)
			.sendContentLength(0)
			.sendEnd();
			this->setFinished();
			Logger::DEBUG() << "Upload finished : " << body.getReceivedSize() << " octets written.";
			Logger::DEBUG() << "File saved here : " << final_filepath;
			body.setFilePath("");
			break;
		}
	case COPYING:
	{
		const size_t CHUNK_SIZE = 65536;
		
		this->srcFile.read(buffer, CHUNK_SIZE);
            std::streamsize bytesRead = this->srcFile.gcount();
			
            if (bytesRead > 0)
            {
				this->dstFile.write(buffer, bytesRead);
                if (this->dstFile.fail())
                {
					this->srcFile.close();
                    this->dstFile.close();
                    std::remove(this->tempPath.c_str());
                    throw HTTPException(HTTPCode::INTERNAL_SERVER_ERROR);
                }
            }
			
            if (this->srcFile.eof())
            {
				Logger::DEBUG() << "Copie asynchrone terminée avec succès !";
                this->srcFile.close();
                this->dstFile.close();
                std::remove(this->tempPath.c_str());
				
                response.sendStatusLine(HTTPCode::CREATED).sendContentLength(0).sendEnd();
                this->setFinished();
                body.setFilePath("");
            }
            break;
        }
	}
}
