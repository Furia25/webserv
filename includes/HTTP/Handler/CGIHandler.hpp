/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 17:08:05 by vdurand           #+#    #+#             */
/*   Updated: 2026/05/13 01:46:06 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _CGIHANDLER_H
# define _CGIHANDLER_H

# include <vector>

# include "Config/Config.hpp"
# include "HTTP/AHandler.hpp"
# include "HTTP/Response.hpp"

class CGIHandler : public AHandler
{
public:
	CGIHandler(
		HTTPHandler& handler,
		Connection& connection,
		const Request& request,
		Body& body,
		const Config::ServerConfig *host_config,
		const Config::RouteConfig *route_config,
		const std::string& physical_path,
		HTTPCode status_code = HTTPCode::OK)
	: AHandler(handler, connection, request, body, host_config, route_config, physical_path, status_code), 
	CGIConfig(static_cast<const Config::CGIConfig&>(*route_config)) {};

	void	onCreation();
	void	onExecute();
	void	initEnvironment();
	void	setEnv(const std::string& key, const std::string& value);

private:
	const Config::CGIConfig&	CGIConfig;

	std::vector<const char *>	env;
};


#endif // _CGIHANDLER_H
