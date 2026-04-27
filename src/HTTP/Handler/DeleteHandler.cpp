/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 11:46:33 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/27 16:26:33 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTP/Handler/DeleteHandler.hpp"

void DeleteHandler::onExecute()
{
	if (!FileSystem::exists(physicalPath))
		throw HTTPException(HTTPCode::NOT_FOUND);
	else if (FileSystem::isDirectory(physicalPath))
		throw HTTPException(HTTPCode::FORBIDDEN);
	else if (!FileSystem::isWritable(physicalPath))
		throw HTTPException(HTTPCode::FORBIDDEN);
	else
	{
		if (std::remove(physicalPath.c_str()) == 0)
			Response::buildEmptyResponse(connection, HTTPCode::NO_CONTENT);
		else
		{
			throw HTTPException(HTTPCode::INTERNAL_SERVER_ERROR);
		}
	}
	this->setFinished();
}
