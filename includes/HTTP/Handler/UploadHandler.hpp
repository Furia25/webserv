/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UploadHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 17:08:05 by vdurand           #+#    #+#             */
/*   Updated: 2026/05/06 17:36:06 by vdurand          ###   ########.fr       */
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
		const Config::ServerConfig *host_config,
		const Config::RouteConfig *route_config,
		const std::string& physical_path, HTTPCode status_code = HTTPCode::OK, bool upload = false)
	: AHandler(handler, connection, request, host_config, route_config, physical_path, status_code),
	uploadConfig(static_cast<const Config::UploadConfig&>(*route_config)), isUpload(upload) {};

	void	cleanTempFile(const std::string& path);
	void	onExecute();
private:
	const Config::UploadConfig&	uploadConfig;
	bool isUpload;
};

#endif // _UPLOADHANDLER_H