/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 11:02:50 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/27 16:03:34 by vdurand          ###   ########.fr       */
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
	Config::ServerConfig *tmpHost = NULL;
	RadixTree<Config::ServerConfig *>::const_iterator it = config.servers.find(hostName);
	if (it == config.servers.end())
	{
		/*IL FAUT PEUT ETRE PRENDRE LE PREMIER SERVEUR c'est pour ca que j'ai mis un iterateur*/
		res.errorCode = HTTPCode::NOT_FOUND;
		return (res);
	}
	res.host = tmpHost;

	if (req.getPath().find("..") != std::string::npos)
	{
		res.errorCode = HTTPCode::FORBIDDEN;
		return (res);
	}

	Config::RouteConfig *tmpRoute = NULL;
	if (!tmpHost->routes.search_prefix(req.getPath(), tmpRoute))
	{
		res.errorCode = HTTPCode::NOT_FOUND;
		return (res);
	}
	res.route = tmpRoute;

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

	/*FAUT AUSSI PRENDRE EN COMPTE LALIAS*/
	res.physicalPath = res.host->root + req.getPath();
	res.success = true;
	return (res);
}