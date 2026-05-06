/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 11:02:50 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/06 03:45:58 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTP/Router.hpp"

static inline void extract_host(const Request &req, std::string& host)
{
	host = "";

	HashMap<std::string, std::string>::const_iterator it = req.getHeaders().find("host");
	if (it == req.getHeaders().end())
		return;

	const std::string&	full_host = it->second;
	size_t				last_colon = full_host.find_last_of(':');
	size_t				closing_bracket = full_host.find_last_of(']');

	if (last_colon != std::string::npos && (closing_bracket == std::string::npos || last_colon > closing_bracket))
		host = full_host.substr(0, last_colon);
	else
		host = full_host;

	if (host.length() >= 2 && host[0] == '[' && host[host.length() - 1] == ']')
		host = host.substr(1, host.length() - 2);
}

Router::RouteResult Router::resolve(const Connection& connection, const Config::AppConfig &config, const Request &req)
{
	Router::RouteResult	res;
	std::string			host;

	extract_host(req, host);
	const RadixTree<Config::ServerConfig *>& tree = config.serversMap.at(connection.getOriginPort());
	RadixTree<Config::ServerConfig *>::const_iterator it = tree.find_prefix(host);
	Config::ServerConfig *tmpHost;
	if (it == tree.end())
	{
		try
		{
			it = tree.begin();
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
