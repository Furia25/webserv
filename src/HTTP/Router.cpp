/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 11:02:50 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/04 17:02:34 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTP/Router.hpp"

static inline std::string extractHost(const Request &req)
{
	HashMap<std::string,
		std::string>::const_iterator it = req.getHeaders().find("host");
	if (it == req.getHeaders().end())
		return ("");

	std::string host = it->second;
	size_t colon_pos = host.find(':');
	if (colon_pos != std::string::npos)
		return (host.substr(0, colon_pos));
	return (host);
}

Router::RouteResult Router::resolve(const Config::AppConfig &config, const Request &req)
{
	Router::RouteResult res;

	std::string hostName = extractHost(req);
	RadixTree<Config::ServerConfig *>::const_iterator it = config.servers.find(hostName);
	Config::ServerConfig *tmpHost;
	if (it == config.servers.end())
	{
		try {
			it = config.servers.begin();
			tmpHost = it->second;
		} catch (const std::exception& e)
		{
			res.errorCode = HTTPCode::NOT_FOUND;
			return (res);
		}
	}
	else
		tmpHost = it->second;
	res.host = tmpHost;

	/*We have to make better checks c'est pas super ça parce que il faut juste pas qu'on dépasse le fichier disque de la route*/
	if (req.getPath().find("..") != std::string::npos)
	{
		res.errorCode = HTTPCode::FORBIDDEN;
		return (res);
	}

	RadixTree<Config::RouteConfig *>::const_iterator iterator = tmpHost->routes.find(req.getPath());
	if (iterator == tmpHost->routes.end())
	{
		res.errorCode = HTTPCode::NOT_FOUND;
		return (res);
	}
	res.route = iterator->second;

	if (res.route->method_allowed[static_cast<size_t>(req.getMethod())] == false)
	{
		res.errorCode = HTTPCode::METHOD_NOT_ALLOWED;
		return (res);
	}

	if (res.host->max_body_size < req.getContentLength())
	{
		res.errorCode = HTTPCode::PAYLOAD_TOO_LARGE;
		return (res);
	}

	if (!res.route->alias.empty())
	{
		const std::string &routePath = iterator->first;
		const std::string &reqPath   = req.getPath();
		std::string remainder = reqPath.substr(routePath.size());
		res.physicalPath = res.host->root + res.route->alias + remainder;
	}
	else
	{
		res.physicalPath = res.host->root + req.getPath();
	}
	res.success = true;
	return (res);
}
