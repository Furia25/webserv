/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UploadHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/05 16:25:18 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/06 12:05:14 by antbonin         ###   ########.fr       */
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
	std::string fileName;
	size_t pos = request.getPath().find_last_of('/');
	if (pos != std::string::npos)
		fileName = request.getPath().substr(pos + 1);
	else
		fileName = request.getPath();

	std::string destination = uploadConfig.upload_store + "/" + fileName + "_" + itoa(connection.getHash());

	if (this->isUpload)
	{
		std::string tempPath = _temp_file_path_  +  itoa(connection.getHash());
		if (std::rename(tempPath.c_str(), destination.c_str()) != 0)
		{
			std::ifstream src(tempPath.c_str(), std::ios::binary);
			std::ofstream dst(destination.c_str(), std::ios::binary);
			if (!src || !dst)
			{
				cleanTempFile(tempPath);
				throw HTTPException(HTTPCode::BAD_GATEWAY);
			}
			dst << src.rdbuf();
			src.close();
			dst.close();
			cleanTempFile(tempPath);
		}
	}
	else
	{
		const std::vector<uint8_t> &body = request.getBody();

		std::ofstream outFile(destination.c_str(), std::ios::binary);
		if (!outFile)
			throw HTTPException(HTTPCode::BAD_GATEWAY);

		outFile.write(reinterpret_cast<const char *>(body.data()), body.size());
		outFile.close();
	}

	Response::buildEmptyResponse(connection, HTTPCode::CREATED);
	this->setFinished();
}
