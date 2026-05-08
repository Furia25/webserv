/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UploadHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/05 16:25:18 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/08 18:54:32 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/Handler/UploadHandler.hpp"
# include "HTTP/HTTPHandler.hpp"
# include "HTTP/HttpTypes.hpp"
# include "Utils/Itoa.hpp"
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

void UploadHandler::onExecute()
{
	if (this->request.getBody().getIsStreaming())
	{
		struct stat st;
		if (stat(request.getBody().getFilePath().c_str(), &st) != 0) 
		{
			throw HTTPException(HTTPCode::INTERNAL_SERVER_ERROR);
		}
		if (static_cast<size_t>(st.st_size) > request.getContentLength())
		{
			std::remove(request.getBody().getFilePath().c_str());
			throw HTTPException(HTTPCode::PAYLOAD_TOO_LARGE);
		}
	}
	else
	{
		const std::vector<uint8_t> &body = request.getBody().getMemoryBuffer();

		std::ofstream outFile(request.getBody().getFilePath().c_str(), std::ios::binary);
		if (!outFile)
			throw HTTPException(HTTPCode::INTERNAL_SERVER_ERROR);

		outFile.write(reinterpret_cast<const char *>(body.data()), body.size());
		outFile.close();
	}

	const_cast<Request&>(request).getBody().setFilePath("");
	Response::buildEmptyResponse(connection, HTTPCode::CREATED);
	connection.setClosing();
	this->setFinished();
}
