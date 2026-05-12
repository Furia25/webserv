/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UploadHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antoine <antoine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 17:08:05 by vdurand           #+#    #+#             */
/*   Updated: 2026/05/12 22:19:05 by antoine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _UPLOADHANDLER_H
# define _UPLOADHANDLER_H

# include "Config/Config.hpp"
# include "HTTP/AHandler.hpp"
# include "HTTP/Response.hpp"

class UploadHandler : public AHandler
{
public:
	UploadHandler(
		HTTPHandler& handler,
		Connection& connection,
		const Request& request,
		Body& body,
		const Config::ServerConfig *host_config,
		const Config::RouteConfig *route_config,
		const std::string& physical_path, HTTPCode status_code = HTTPCode::OK)
	: AHandler(handler, connection, request, body, host_config, route_config, physical_path, status_code)
	{
	};

	void	cleanTempFile(const std::string& path);
	void	onExecute();
private:
};

#endif // _UPLOADHANDLER_H