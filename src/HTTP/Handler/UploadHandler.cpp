/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UploadHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/05 16:25:18 by antbonin          #+#    #+#             */
/*   Updated: 2026/06/06 15:26:04 by antbonin         ###   ########.fr       */
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
		if (this->body.getIsStreaming() && this->body.getFileWriter())
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
		if (body.getIsStreaming()) 
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
	std::string current_filepath = body.getFilePath();
	std::string final_filepath = current_filepath;
	std::string content_type_val;
	if (body.getIsStreaming() && current_filepath.size() > 4 && current_filepath.substr(current_filepath.size() - 4) == ".tmp") 
		final_filepath = current_filepath.substr(0, current_filepath.size() - 4);
	HashMap<std::string, std::string>::const_iterator it = this->request.getHeaders().find("content-type");
	if (it != this->request.getHeaders().end())
    	content_type_val = it->second;
	if (!isExtensionAllowed(final_filepath, content_type_val ,this->uploadConfig.allowed_extensions)) 
	{
		if (body.getIsStreaming()) std::remove(current_filepath.c_str());
		throw HTTPException(HTTPCode::UNSUPPORTED_MEDIA);
	}
	if (FileSystem::exists(final_filepath) && !this->uploadConfig.allow_overwrite)
	{
		if (body.getIsStreaming()) std::remove(current_filepath.c_str());
		throw HTTPException(HTTPCode::CONFLICT);
	}
	if (body.getIsStreaming())
	{
		struct stat st;
		if (stat(current_filepath.c_str(), &st) != 0) 
		{
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
	}
	else
	{
		const std::vector<uint8_t> &bod = this->body.getMemoryBuffer();
		if (this->uploadConfig.max_body_size > 0 && bod.size() > this->uploadConfig.max_body_size)
			throw HTTPException(HTTPCode::PAYLOAD_TOO_LARGE);
		std::ofstream outFile(final_filepath.c_str(), std::ios::binary | std::ios::trunc);
		if (!outFile)
		{
			throw HTTPException(HTTPCode::INTERNAL_SERVER_ERROR);
		}
		outFile.write(reinterpret_cast<const char *>(bod.data()), bod.size());
		outFile.close();
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
}
