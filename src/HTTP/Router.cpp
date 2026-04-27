/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 11:02:50 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/27 11:36:02 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTP/RouteResult.hpp"
#include "HTTP/Router.hpp"

Router::Router(const Config::AppConfig &config)
	: _config(config)
{
}

std::string Router::extractHost(const Request &req) const
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

RouteResult Router::resolve(const Request &req) const
{
	RouteResult res;

	std::string hostName = extractHost(req);

	Config::ServerConfig *tmpHost = NULL;
	if (!_config.servers.search(hostName, tmpHost))
	{
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

	int methodIndex = static_cast<int>(req.getMethod());
	if (methodIndex < 0 || methodIndex >= 6
		|| res.route->method_allowed[methodIndex] == false)
	{
		res.errorCode = HTTPCode::METHOD_NOT_ALLOWED;
		return (res);
	}

	if (res.host->max_body_size < req.getContentLength())
	{
		res.errorCode = HTTPCode::PAYLOAD_TOO_LARGE;
		return (res);
	}

	res.physicalPath = res.host->root + req.getPath();
	res.success = true;
	return (res);
}
