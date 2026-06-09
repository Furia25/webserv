/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UploadHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/05 16:25:18 by antbonin          #+#    #+#             */
/*   Updated: 2026/06/09 15:42:29 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/Utils/GenerateUniqueFilename.hpp"
# include "HTTP/HTTPHandler.hpp"
# include "HTTP/HTTPTypes.hpp"
# include "HTTP/Handler/UploadHandler.hpp"
# include "Utils/FileSystem.hpp"
# include "Utils/IntegerUtils.hpp"
# include <stdio.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <unistd.h>

void UploadHandler::onCreation()
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
	if (this->uploadConfig.max_body_size > 0
		&& this->request.content_length > this->uploadConfig.max_body_size)
	{
		Logger::ERROR() << "Upload Payload too large in headers";
		std::remove(body.getFilePath().c_str());
		throw HTTPException(HTTPCode::PAYLOAD_TOO_LARGE);
	}
}

bool	isExtensionAllowed(const std::string &filename, const std::string &contentType, const std::vector<MIME> &allowed_exts)
{
	bool	is_ok;
	size_t	semi_colon;
	MIME	content_mime;
	size_t	dotPos;
	MIME	file_mime;

	if (allowed_exts.empty())
		return (true);
	is_ok = false;
	if (!contentType.empty())
	{
		std::string clean_type = contentType;
		semi_colon = clean_type.find(";");
		if (semi_colon != std::string::npos)
			clean_type = clean_type.substr(0, semi_colon);
		content_mime = MIME::from_extension(clean_type);
		for (size_t i = 0; i < allowed_exts.size(); ++i)
		{
			if (content_mime == allowed_exts[i])
				is_ok = true;
		}
		if (!is_ok)
			return (false);
	}
	dotPos = filename.find_last_of('.');
	if (dotPos == std::string::npos)
		return (false);
	std::string ext = filename.substr(dotPos + 1);
	file_mime = MIME::from_extension(ext);
	for (size_t i = 0; i < allowed_exts.size(); ++i)
	{
		if (file_mime == allowed_exts[i])
			return true;
	}
	return false;
}

void UploadHandler::onExecute()
{
	if (this->body.getFileWriter())
		this->body.getFileWriter()->close();

	std::string current_filepath = body.getFilePath();
	std::string filename = "";
	std::string extension = ".bin";
	size_t last_slash = this->request.path.find_last_of('/');
	if (last_slash != std::string::npos)
	{
		std::string potential_name = this->request.path.substr(last_slash + 1);
		if (potential_name.find('.') != std::string::npos)
			filename = potential_name;
	}
	
	std::string content_type_val;
	HashMap<std::string, std::string>::const_iterator it = this->request.getHeaders().find("content-type");
	if (it != this->request.getHeaders().end())
		content_type_val = it->second;
	extension = MIME::to_extension(MIME::from_extension(content_type_val));
		
	std::string final_filepath = uploadConfig.alias;
	if (!final_filepath.empty() && final_filepath[final_filepath.length() - 1] != '/')
		final_filepath += "/";
	if (filename.empty())
		filename = "file_" + GenerateUniqueFilename("") + extension;
	if (!isExtensionAllowed(filename, content_type_val, this->uploadConfig.allowed_extensions))
	{
		std::remove(current_filepath.c_str());
		throw HTTPException(HTTPCode::UNSUPPORTED_MEDIA);
	}
	final_filepath += filename;
	if (FileSystem::exists(final_filepath) && !this->uploadConfig.allow_overwrite)
	{
		std::remove(current_filepath.c_str());
		throw HTTPException(HTTPCode::CONFLICT);
	}
	if (this->uploadConfig.max_body_size > 0 && body.getSize() > this->uploadConfig.max_body_size)
	{
		Logger::ERROR() << "Upload de-chunked too big for config : " << body.getSize();
		std::remove(current_filepath.c_str());
		throw HTTPException(HTTPCode::PAYLOAD_TOO_LARGE);
	}
	if (std::rename(current_filepath.c_str(), final_filepath.c_str()) != 0)
	{
		std::remove(current_filepath.c_str());
		throw HTTPException(HTTPCode::INTERNAL_SERVER_ERROR);
	}
	response.sendStatusLine(HTTPCode::CREATED).sendDefaults(this->request,
		this->routeResult.route).sendContentLength(0).sendEnd();
	this->setFinished();
	Logger::DEBUG() << "Upload finished : " << body.getReceivedSize() << " octets written.";
	Logger::DEBUG() << "File saved here : " << final_filepath;
	body.setFilePath("");
}
