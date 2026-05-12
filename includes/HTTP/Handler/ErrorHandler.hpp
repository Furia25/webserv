/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorHandler.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antoine <antoine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 17:54:35 by vdurand           #+#    #+#             */
/*   Updated: 2026/05/12 22:19:59 by antoine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _ERRORHANDLER_H
# define _ERRORHANDLER_H

# include "Config/Config.hpp"
# include "HTTP/AHandler.hpp"
# include "HTTP/Response.hpp"

class ErrorHandler : public AHandler
{
public:
	ErrorHandler(
		HTTPHandler& handler,
		Connection& connection,
		const Request& request,
		Body& body,
		const Config::ServerConfig *host_config,
		const Config::RouteConfig *route_config,
		const std::string& physical_path,
		HTTPCode status_code = HTTPCode::INTERNAL_SERVER_ERROR)
	: AHandler(handler, connection, request, body, host_config, route_config, physical_path, status_code), state(INIT) {}

	void	onExecute();
private:
	enum State
	{
		INIT,
		SEND_HEADERS,
		SEND_BODY,
		SEND_DEFAULT_ERROR,
		FINISHED
	};
	FileReader	fileReader;
	State		state;
};

#endif // _ERRORHANDLER_H