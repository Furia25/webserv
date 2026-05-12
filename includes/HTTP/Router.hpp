/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antoine <antoine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 11:01:45 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/13 00:30:39 by antoine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _ROUTER_H
# define _ROUTER_H

# include "Server/AddressResolver.hpp"
# include "Config/Config.hpp"
# include "HTTP/Request.hpp"
# include "HTTP/HttpTypes.hpp"
# include "Server/Connection.hpp"
# include "HTTP/URIUtils.hpp"

namespace Router 
{
	struct RouteResult
	{
		const Config::ServerConfig	*host;
		const Config::RouteConfig	*route;
		std::string					physicalPath;
		HTTPCode					errorCode;
		bool						success;

		RouteResult() : host(NULL), route(NULL), errorCode(HTTPCode::OK), success(false) {}
	};

	const Config::ServerConfig* matchServer(const Connection& connection, const Config::AppConfig &config, const Request &request);
	const Config::RouteConfig*	matchRoute(const Config::ServerConfig* host, const std::string& current_path);
	void						validateConstraints(const Config::ServerConfig* host, const Config::RouteConfig* route, const Request& request);
	std::string					Router::buildPhysicalPath(const Config::ServerConfig* host, const Config::RouteConfig* route, const std::string& current_path);

	RouteResult resolve(const Connection& connection, const Config::AppConfig &config, const Request &req);
	const Config::ServerConfig&	findDefaultServer(port_t port, const Config::AppConfig &config);
};

class RouterException : public std::exception 
{
private:
	HTTPCode code;
public:
	RouterException(HTTPCode code) : code(code) {}
	virtual ~RouterException() throw() {}
	HTTPCode getCode() const { return code; }
};

#endif // _ROUTER_H