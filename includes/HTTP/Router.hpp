/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 11:01:45 by antbonin          #+#    #+#             */
/*   Updated: 2026/06/08 14:51:54 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _ROUTER_H
# define _ROUTER_H

# include "Server/AddressResolver.hpp"
# include "Config/Config.hpp"
# include "HTTP/Request.hpp"
# include "HTTP/HTTPTypes.hpp"
# include "Server/Connection.hpp"
# include "HTTP/Utils/URIUtils.hpp"

namespace Router 
{
	struct RouteResult
	{
		const Config::ServerConfig	*host;
		const Config::RouteConfig	*route;
		std::string					hostStr;
		bool						success;
		std::string					basePath;
		std::string					fullPath;
		HTTPCode					errorCode;
		std::string					pathRemainder;

		RouteResult() : host(NULL), route(NULL), success(false), errorCode(HTTPCode::OK) {}
	};

	RouteResult						resolve(const Connection& connection, const Config::AppConfig &config, const Request &req);
	const Config::ServerConfig&		findDefaultServer(port_t port, const Config::AppConfig &config);
};


#endif // _ROUTER_H