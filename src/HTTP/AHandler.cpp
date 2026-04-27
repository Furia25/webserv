/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AHandler.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 15:57:58 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/27 16:59:45 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "AHandler.hpp"
#include "Utils/FileSystem.hpp"

bool AHandler::execute()
{
	if (this->statusCode != HTTPCode::OK)
	{
		try { this->handleError(); }
		catch (const std::exception& e)
		{
			this->setFinished();
			this->connection.setDeletable();
			Logger::ERROR() << "Unable to recover error for connection: " << this->connection << "\n\t" << e.what();
		}
		
		return !this->finished;
	}

	try
	{
		this->onExecute();
	}
	catch (const HTTPException& http_exception)
	{
		this->statusCode = http_exception.getStatusCode();
		this->initError();
	}
	catch (const std::exception& e)
	{
		this->statusCode = HTTPCode::INTERNAL_SERVER_ERROR;
		this->initError();
	}
	return !this->finished;
}

void AHandler::handleError()
{
	/*Read File brick by brick*/
}

void AHandler::initError()
{
	HashMap<HTTPCode, std::string>::const_iterator it = hostConfig.error_fallbacks.find(this->statusCode);
	if (it != hostConfig.error_fallbacks.end())
	{
		std::string error_path = it->second;
		if (error_path.length() > 0 && error_path[0] != '/')
			error_path = hostConfig.root + "/" + error_path;
		else
			error_path = hostConfig.root + error_path;

		if (FileSystem::exists(error_path) && FileSystem::isFile(error_path) && FileSystem::isReadable(error_path))
		{
			this->physicalPath = error_path;
			return;
		}
	}
	Response::buildErrorResponse(connection, this->statusCode);
}