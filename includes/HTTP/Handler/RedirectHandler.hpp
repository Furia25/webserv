/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RedirectHandler.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 17:08:05 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/27 17:55:09 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _REDIRECTHANDLER_H
# define _REDIRECTHANDLER_H

# include "Config/Config.hpp"
# include "HTTP/AHandler.hpp"
# include "HTTP/Response.hpp"

class RedirectHandler : public AHandler
{
public:
	RedirectHandler(
		Connection& connection,
		const Request& request,
		const Config::ServerConfig& host_config,
		const Config::RouteConfig& route_config,
		const std::string& physical_path,
		HTTPCode status_code = HTTPCode::OK)
	: AHandler(connection, request, host_config, route_config, physical_path, status_code), 
	redirectConfig(static_cast<const Config::RedirectConfig&>(route_config)) {};

	void	onExecute();
private:
	const Config::RedirectConfig&	redirectConfig;
};

#endif // _REDIRECTHANDLER_H