/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 11:01:45 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/13 01:58:52 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _ROUTER_H
# define _ROUTER_H

# include "Server/AddressResolver.hpp"
# include "Config/Config.hpp"
# include "HTTP/Request.hpp"
# include "HTTP/HttpTypes.hpp"
# include "Server/Connection.hpp"
# include "Utils/HTTP/URIUtils.hpp"

namespace Router 
{
	struct RouteResult
	{
		const Config::ServerConfig	*host;
		const Config::RouteConfig	*route;
		std::string					physicalPath;
		std::string					pathRemainder;
		HTTPCode					errorCode;
		bool						success;

		RouteResult() : host(NULL), route(NULL), errorCode(HTTPCode::OK), success(false) {}
	};

	const Config::ServerConfig	*matchServer(const Connection& connection, const Config::AppConfig& config, const Request& request);
	const Config::RouteConfig	*matchRoute(const Config::ServerConfig *host, const std::string& current_path);
	void						validateConstraints(const Config::ServerConfig *host, const Config::RouteConfig *route, const Request& request);
	std::string					buildPhysicalPath(const Config::ServerConfig *host, const Config::RouteConfig *route, const std::string& current_path);

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