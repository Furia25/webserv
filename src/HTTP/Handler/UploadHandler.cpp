/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UploadHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/05 16:25:18 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/11 01:43:35 by vdurand          ###   ########.fr       */
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

void UploadHandler::onExecute()
{
	std::string fileName;
	size_t pos = request.path.find_last_of('/');
	if (pos != std::string::npos)
		fileName = request.path.substr(pos + 1);
	else
		fileName = request.path;

	std::string destination = uploadConfig.upload_store + "/"
		+ fileName + "_" + IntegerUtils::itoa(connection.getHash());

	if (this->isUpload)
	{
		std::string tempPath = _temp_file_path_ + IntegerUtils::itoa(connection.getHash());
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
