/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 11:46:33 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/27 12:46:24 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTP/DeleteHandler.hpp"

DeleteHandler::DeleteHandler(Connection &conn, const std::string &path)
	: connection(conn), physical_path(path), isFinished(false)
{
}

bool DeleteHandler::execute()
{
	if (!FileSystem::exists(physical_path))
		Response::buildErrorResponse(connection, HTTPCode::NOT_FOUND);
	else if (FileSystem::isDirectory(physical_path))
		Response::buildErrorResponse(connection, HTTPCode::FORBIDDEN);
	else if (!FileSystem::isWritable(physical_path))
		Response::buildErrorResponse(connection, HTTPCode::FORBIDDEN);
	else
	{
		if (std::remove(physical_path.c_str()) == 0)
			Response::buildEmptyResponse(connection, HTTPCode::NO_CONTENT);
		else
		{
			Response::buildErrorResponse(connection, HTTPCode::INTERNAL_SERVER_ERROR);
		}
	}
	isFinished = true;
	return (true);
}
