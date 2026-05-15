/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UploadHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/05 16:25:18 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/15 05:30:01 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/Handler/UploadHandler.hpp"
# include "HTTP/HTTPHandler.hpp"
# include "HTTP/HTTPTypes.hpp"
# include "Utils/IntegerUtils.hpp"
# include <stdio.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/stat.h>

void	UploadHandler::cleanTempFile(const std::string& path)
{
	struct stat buffer;
	if (stat(path.c_str(), &buffer) == 0)
	{
		if	(std::remove(path.c_str()) != 0)
			throw HTTPException(HTTPCode::INTERNAL_SERVER_ERROR);
	}
}

void	UploadHandler::onExecute()
{
	//this->uploadConfig.allow_overwrite a gerer
	//this->uploadConfig.allowed_extensions a gerer aussi
	if (body.getIsStreaming())
	{
		struct stat st;
		if (stat(body.getFilePath().c_str(), &st) != 0) 
			throw HTTPException(HTTPCode::INTERNAL_SERVER_ERROR);
		if (this->uploadConfig.max_body_size > 0 && static_cast<size_t>(st.st_size) > this->uploadConfig.max_body_size)
		{
			Logger::ERROR() << "Upload de-chunked too big for config : " << st.st_size;
			std::remove(body.getFilePath().c_str());
			throw HTTPException(HTTPCode::PAYLOAD_TOO_LARGE);
		}
	}
	else
	{
		const std::vector<uint8_t> &bod = this->body.getMemoryBuffer();
		if (this->uploadConfig.max_body_size > 0 && bod.size() > this->uploadConfig.max_body_size)
			throw HTTPException(HTTPCode::PAYLOAD_TOO_LARGE);
		std::ofstream outFile(body.getFilePath().c_str(), std::ios::binary);
		if (!outFile)
			throw HTTPException(HTTPCode::INTERNAL_SERVER_ERROR);
		outFile.write(reinterpret_cast<const char *>(bod.data()), bod.size());
		outFile.close();
	}
	response
		.sendStatusLine(HTTPCode::CREATED)
		.sendDefaults(this->request, *this->routeResult.route)
		.sendContentLength(0)
		.sendEnd();
	this->setFinished();
	Logger::DEBUG() << "Upload finished : " << body.getReceivedSize() << " octets written.";
	Logger::DEBUG() << "Files saved here : " << body.getFilePath();
	body.setFilePath("");
}
