/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 11:02:50 by antbonin          #+#    #+#             */
/*   Updated: 2026/06/06 20:27:38 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTP/Router.hpp"
#include "Utils/IntegerUtils.hpp"

inline void extract_host(const Request &request, std::string& host)
{
	host = "";

	Headers::const_iterator it = request.getHeaders().find("host");
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

static inline void match_server(Router::RouteResult& result, const Connection& connection,
		const Config::AppConfig &config, const Request &request)
{
	extract_host(request, result.hostStr);
	const RadixTree<Config::ServerConfig *>& tree = config.serversMap.at(connection.getOriginPort());
	RadixTree<Config::ServerConfig *>::const_iterator it = tree.find_prefix(result.hostStr);
	if (it == tree.end())
		result.host = &Router::findDefaultServer(connection.getOriginPort(), config);
	else
		result.host = it->second;
}

static inline void match_route(Router::RouteResult& result, const std::string& current_path)
{
	RadixTree<Config::RouteConfig *>::const_iterator route_it = result.host->routes.find_prefix(current_path);
	if (route_it == result.host->routes.end())
		throw RouterException(HTTPCode::NOT_FOUND);
	result.route = route_it->second;
}

static inline void	validate_constraints(Router::RouteResult& result, const Request& request)
{
	if (result.host->max_body_size < request.content_length)
		throw RouterException(HTTPCode::PAYLOAD_TOO_LARGE);

	if (result.route->method_allowed[static_cast<size_t>(request.method)] == false)
		throw RouterException(HTTPCode::METHOD_NOT_ALLOWED);

	const Cookies request_cookies = request.getCookies();
	const HashMap<std::string, Config::CookieConfig>& route_cookies = result.route->cookies;

	for (HashMap<std::string, Config::CookieConfig>::const_iterator it = route_cookies.begin(); it != route_cookies.end(); ++it)
	{
		if (it->second.required && !request_cookies.contain(it->first))
		throw RouterException(HTTPCode::FORBIDDEN);
	}

	if (result.route->handler == HandlerType::UPLOAD)
	{
		const Config::UploadConfig* upload_route = static_cast<const Config::UploadConfig*>(result.route);
		if (upload_route->upload_store.empty())
			throw RouterException(HTTPCode::FORBIDDEN);
	}
}

static inline void	build_physical_path(Router::RouteResult& result, const std::string& current_path)
{
	const Config::ServerConfig	*host = result.host;
	const Config::RouteConfig	*route = result.route;
	RadixTree<Config::RouteConfig *>::const_iterator route_it = host->routes.find_prefix(current_path);
	const std::string&	found_path = route_it->first;
	std::string			remainder = current_path.substr(found_path.size());
	if (!remainder.empty() && remainder[0] == '/' && !found_path.empty() && found_path[found_path.size() - 1] == '/')
		remainder = remainder.substr(1);
	std::string base_physical_path;
	std::string jail_dir;
	if (!route->alias.empty())
	{
		base_physical_path = route->alias; 
		jail_dir = route->alias;           
	}
	else
	{
		base_physical_path = route->root + found_path;
		jail_dir = route->root;
	}
	base_physical_path = URIUtils::normalizePath(base_physical_path);
	std::string root_jail = URIUtils::normalizePath(jail_dir); 
	result.fullPath = base_physical_path + remainder;
	result.fullPath = URIUtils::normalizePath(result.fullPath);
	if (result.fullPath.find(root_jail) != 0)
		throw RouterException(HTTPCode::FORBIDDEN);
	result.basePath = base_physical_path;
	result.pathRemainder = remainder;
}

Router::RouteResult Router::resolve(const Connection& connection, const Config::AppConfig &config, const Request &request)
{
	Router::RouteResult res;

	res.success = false;

	try 
	{
		std::string decoded_uri = URIUtils::decodeURI(request.path);
        std::string clean_uri = URIUtils::normalizePath(decoded_uri);
		match_server(res, connection, config, request);
		match_route(res, clean_uri);

		validate_constraints(res, request);
		build_physical_path(res, clean_uri);

		res.success = true;
	}
	catch (const RouterException& e)
	{
		res.success = false;
		res.errorCode = e.getCode();
	}

	return res;
}

const Config::ServerConfig& Router::findDefaultServer(port_t port, const Config::AppConfig &config)
{
	const RadixTree<Config::ServerConfig *>& tree = config.serversMap.at(port);
	if (tree.begin() == tree.end())
		throw std::runtime_error("Unable to find default server for port" + IntegerUtils::itoa(port));
	return *tree.begin()->second;
}
