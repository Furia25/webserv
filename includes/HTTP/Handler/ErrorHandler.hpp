/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorHandler.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 17:54:35 by vdurand           #+#    #+#             */
/*   Updated: 2026/05/06 17:35:07 by vdurand          ###   ########.fr       */
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
		const Config::ServerConfig *host_config,
		const Config::RouteConfig *route_config,
		const std::string& physical_path,
		HTTPCode status_code = HTTPCode::INTERNAL_SERVER_ERROR)
	: AHandler(handler, connection, request, host_config, route_config, physical_path, status_code), state(INIT) {}

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