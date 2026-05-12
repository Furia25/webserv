/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antoine <antoine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 11:02:50 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/13 00:32:02 by antoine          ###   ########.fr       */
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

const Config::ServerConfig* Router::matchServer(const Connection& connection, const Config::AppConfig &config, const Request &request)
{
	std::string	host;
	extract_host(request, host);
	const RadixTree<Config::ServerConfig *>& tree = config.serversMap.at(connection.getOriginPort());
	RadixTree<Config::ServerConfig *>::const_iterator it = tree.find_prefix(host);
	if (it == tree.end())
		return &findDefaultServer(connection.getOriginPort(), config);
	return it->second;
}

const Config::RouteConfig* Router::matchRoute(const Config::ServerConfig* host, const std::string& current_path)
{
    RadixTree<Config::RouteConfig *>::const_iterator route_it = host->routes.find_prefix(current_path);
    if (route_it == host->routes.end())
        throw RouterException(HTTPCode::NOT_FOUND);
    return route_it->second;
}

void	Router::validateConstraints(const Config::ServerConfig* host, const Config::RouteConfig* route, const Request& request)
{
	if (host->max_body_size < request.content_length)
		throw RouterException(HTTPCode::PAYLOAD_TOO_LARGE);

	if (route->method_allowed[static_cast<size_t>(request.method)] == false)
		throw RouterException(HTTPCode::METHOD_NOT_ALLOWED);

	const Request::Cookies request_cookies = request.getCookies();
	const HashMap<std::string, Config::CookieConfig>& route_cookies = route->cookies;

	for (HashMap<std::string, Config::CookieConfig>::const_iterator it = route_cookies.begin(); it != route_cookies.end(); ++it)
	{
		if (it->second.required && !request_cookies.contain(it->first))
		throw RouterException(HTTPCode::FORBIDDEN);
	}
}

std::string	Router::buildPhysicalPath(const Config::ServerConfig* host, const Config::RouteConfig* route, const std::string& current_path)
{
	std::string decoded_path = decodeURI(current_path);
	RadixTree<Config::RouteConfig *>::const_iterator route_it = host->routes.find_prefix(decoded_path);
	const std::string& found_path = route_it->first;
	std::string remainder = decoded_path.substr(found_path.size());
	if (!remainder.empty() && remainder[0] == '/' && !found_path.empty() && found_path[found_path.size() - 1] == '/')
		remainder = remainder.substr(1);
	std::string physicalPath;
	if (!route->alias.empty())
		physicalPath = host->root + route->alias + remainder;
	else
		physicalPath = host->root + found_path + remainder;
	std::string finalPath = normalizePath(physicalPath);
	std::string rootJail = normalizePath(host->root); 
	if (finalPath.find(rootJail) != 0) 
		throw RouterException(HTTPCode::FORBIDDEN); 
	return finalPath;
}

Router::RouteResult Router::resolve(const Connection& connection, const Config::AppConfig &config, const Request &request)
{
    Router::RouteResult res;
    res.success = false;

    try 
    {
        res.host = matchServer(connection, config, request);
        res.route = matchRoute(res.host, request.path);
        
        validateConstraints(res.host, res.route, request);
        
        res.physicalPath = buildPhysicalPath(res.host, res.route, request.path);
        
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
