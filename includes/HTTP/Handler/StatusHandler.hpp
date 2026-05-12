/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StatusHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antoine <antoine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 17:08:05 by vdurand           #+#    #+#             */
/*   Updated: 2026/05/12 22:19:18 by antoine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _STATUSHANDLER_H
# define _STATUSHANDLER_H

# include "Config/Config.hpp"
# include "HTTP/AHandler.hpp"
# include "HTTP/Response.hpp"

class StatusHandler : public AHandler
{
public:
	StatusHandler(
		HTTPHandler& handler,
		Connection& connection,
		const Request& request,
		Body& body,
		const Config::ServerConfig *host_config,
		const Config::RouteConfig *route_config,
		const std::string& physical_path,
		HTTPCode status_code = HTTPCode::OK)
	: AHandler(handler, connection, request, body, host_config, route_config, physical_path, status_code), 
	statusConfig(static_cast<const Config::StatusConfig&>(*route_config)) {};

	void	onExecute();
private:
	const Config::StatusConfig&	statusConfig;
};


#endif // _STATUSHANDLER_H