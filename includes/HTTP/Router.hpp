/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 11:01:45 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/28 13:52:08 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _ROUTER_H
# define _ROUTER_H

# include "Config/Config.hpp"
# include "HTTP/Request.hpp"
# include "HTTP/HttpTypes.hpp"

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

	RouteResult resolve(const Config::AppConfig &config, const Request &req);
};

#endif // _ROUTER_H