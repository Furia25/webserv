/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UploadHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/05 16:25:18 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/19 15:49:52 by antbonin         ###   ########.fr       */
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

bool	isExtensionAllowed(const std::string& filename, const std::vector<MIME>& allowed_exts) 
{
	if (allowed_exts.empty()) 
		return true;
	size_t dotPos = filename.find_last_of('.');
	if (dotPos == std::string::npos) 
		return false;
	std::string ext = filename.substr(dotPos + 1);
	for (size_t i = 0; i < allowed_exts.size(); ++i) 
	{
		if (MIME::from_extension(ext) == allowed_exts[i])
			return true;
	}
	return false;
}

void    UploadHandler::onExecute()
{
	std::string current_filepath = body.getFilePath();
	std::string final_filepath = current_filepath;

	if (body.getIsStreaming() && current_filepath.size() > 4 && current_filepath.substr(current_filepath.size() - 4) == ".tmp") 
		final_filepath = current_filepath.substr(0, current_filepath.size() - 4);
	if (!isExtensionAllowed(final_filepath, this->uploadConfig.allowed_extensions)) 
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
