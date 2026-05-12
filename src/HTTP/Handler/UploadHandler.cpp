/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UploadHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antoine <antoine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/05 16:25:18 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/12 22:24:23 by antoine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/Handler/UploadHandler.hpp"
# include "HTTP/HTTPHandler.hpp"
# include "HTTP/HttpTypes.hpp"
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
	size_t maxSize = CONFIG_BODY_SIZE;
	if (body.getIsStreaming())
	{
		struct stat st;
		if (stat(body.getFilePath().c_str(), &st) != 0) 
			throw HTTPException(HTTPCode::INTERNAL_SERVER_ERROR);
		if (maxSize > 0 && static_cast<size_t>(st.st_size) > maxSize)
		{
			Logger::ERROR() << "Upload de-chunked trop gros pour la config : " << st.st_size;
			std::remove(body.getFilePath().c_str());
			throw HTTPException(HTTPCode::PAYLOAD_TOO_LARGE);
		}
	}
	else
	{
		const std::vector<uint8_t> &bod = this->body.getMemoryBuffer();
		if (maxSize > 0 && bod.size() > maxSize)
			throw HTTPException(HTTPCode::PAYLOAD_TOO_LARGE);
		std::ofstream outFile(body.getFilePath().c_str(), std::ios::binary);
		if (!outFile)
			throw HTTPException(HTTPCode::INTERNAL_SERVER_ERROR);
		outFile.write(reinterpret_cast<const char *>(bod.data()), bod.size());
		outFile.close();
	}
	Response::buildEmptyResponse(connection, HTTPCode::CREATED);
	connection.setClosing();
	this->setFinished();
	Logger::INFO() << "Upload finished : " << body.getReceivedSize() << " octets written.";
	Logger::INFO() << "Files saved here : " << body.getFilePath();
	body.setFilePath("");
}
