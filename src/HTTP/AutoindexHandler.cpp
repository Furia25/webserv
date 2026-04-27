/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AutoindexHandler.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 10:45:48 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/27 10:58:31 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTP/AutoindexHandler.hpp"

AutoindexHandler::AutoindexHandler(const Request req, Connection &conn, const std::string& path): request(req), connection(conn), physical_path(path), isFinished(false)
{
}

bool AutoindexHandler::execute()
{
	std::string path = request.getPath();
	std::string body = "<html><head><title>Index of " + path
		+ "</title></head>";
	body += "<body><h1>Index of " + path + "</h1><hr><ul>";

	DIR *dir = opendir(physical_path.c_str());
	if (dir)
	{
		struct dirent *ent;
		while ((ent = readdir(dir)) != NULL)
		{
			std::string name = ent->d_name;
			if (name == ".")
				continue ;

			std::string sep = (path.empty() || path[path.length()
					- 1] == '/') ? "" : "/";
			std::string link = path + sep + name;
			body += "<li><a href=\"" + link + "\">" + name + "</a></li>";
		}
		closedir(dir);
		body += "</ul><hr></body></html>";

		Response::buildRawResponse(connection, HTTPCode::OK, "text/html", body);
	}
	else
	{
		Response::buildErrorResponse(connection, HTTPCode::FORBIDDEN);
	}

	isFinished = true;
	return (true);
}
