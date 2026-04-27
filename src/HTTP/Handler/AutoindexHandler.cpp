/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AutoindexHandler.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 10:45:48 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/27 17:26:09 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTP/Handler/AutoindexHandler.hpp"

void AutoindexHandler::onExecute()
{
	/*CA CEST PAS OUF*/
	std::string path = request.getPath();
	std::string body = "<html><head><title>Index of " + path
		+ "</title></head>";
	body += "<body><h1>Index of " + path + "</h1><hr><ul>";

	DIR *dir = opendir(physicalPath.c_str());
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

		Response::buildRawResponse(connection, HTTPCode::OK, MIME::html, body);
	}
	else
		throw HTTPException(HTTPCode::FORBIDDEN);
	this->setFinished();
}
