/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 11:02:50 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/11 02:34:15 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTP/Router.hpp"
#include "Utils/IntegerUtils.hpp"

static inline void extract_host(const Request &request, std::string& host)
{
	host = "";

	HashMap<std::string, std::string>::const_iterator it = request.getHeaders().find("host");
	if (it == request.getHeaders().end())
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

# define ROUTER_ERROR(code) do { res.errorCode = code; return (res); } while (0)

/*Il fait trop de chose le pelo zbi*/
Router::RouteResult Router::resolve(const Connection& connection, const Config::AppConfig &config, const Request &request)
{
	Router::RouteResult	res;
	std::string			host;

	extract_host(request, host);
	const RadixTree<Config::ServerConfig *>& tree = config.serversMap.at(connection.getOriginPort());
	RadixTree<Config::ServerConfig *>::const_iterator it = tree.find_prefix(host);
	const Config::ServerConfig *tmpHost;
	if (it == tree.end())
		tmpHost = &Router::findDefaultServer(connection.getOriginPort(), config);
	else
		tmpHost = it->second;
	res.host = tmpHost;

	/*We have to make better checks c'est pas super ça parce que il faut juste pas qu'on dépasse le fichier disque de la route*/
	if (request.path.find("../") != std::string::npos)
		ROUTER_ERROR(HTTPCode::FORBIDDEN);

	const std::string& current_path = request.path;
	RadixTree<Config::RouteConfig *>::const_iterator route_it = tmpHost->routes.find_prefix(current_path);
	if (route_it == tmpHost->routes.end())
		ROUTER_ERROR(HTTPCode::NOT_FOUND);
	res.route = route_it->second;

	/*Check for missing required cookies*/
	const Request::Cookies	request_cookies = request.getCookies();
	const HashMap<std::string, Config::CookieConfig>&	route_cookies = res.route->cookies;

	for (HashMap<std::string, Config::CookieConfig>::const_iterator it = route_cookies.begin();
			it != route_cookies.end(); ++it)
	{
		if (it->second.required && !request_cookies.contain(it->first))
			ROUTER_ERROR(HTTPCode::FORBIDDEN);
	}

	if (res.route->method_allowed[static_cast<size_t>(request.method)] == false)
		ROUTER_ERROR(HTTPCode::METHOD_NOT_ALLOWED);
	if (res.host->max_body_size < request.content_length)
		ROUTER_ERROR(HTTPCode::PAYLOAD_TOO_LARGE);

	/*Route resolving from alias / root*/
	const std::string& found_path = route_it->first;
	std::string remainder = current_path.substr(found_path.size());
	if (!remainder.empty() && remainder[0] == '/' && !found_path.empty() && found_path[found_path.size() - 1] == '/')
		remainder = remainder.substr(1);

	if (!res.route->alias.empty())
		res.physicalPath = res.host->root + res.route->alias + remainder;
	else
		res.physicalPath = res.host->root + found_path + remainder;

	res.success = true;
	return res;
}

const Config::ServerConfig& Router::findDefaultServer(port_t port, const Config::AppConfig &config)
{
	const RadixTree<Config::ServerConfig *>& tree = config.serversMap.at(port);
	if (tree.begin() == tree.end())
		throw std::runtime_error("Unable to find default server for port" + IntegerUtils::itoa(port));
	return *tree.begin()->second;
}
