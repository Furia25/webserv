/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StatusHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 17:08:05 by vdurand           #+#    #+#             */
/*   Updated: 2026/05/15 04:22:13 by vdurand          ###   ########.fr       */
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
		const Router::RouteResult& route_result,
		HTTPCode status_code = HTTPCode::OK)
	: AHandler(handler, connection, request, body, route_result, status_code), 
	statusConfig(static_cast<const Config::StatusConfig&>(*route_result.route)) {};

	void	onExecute() {};
	void	onCreation();
private:
	const Config::StatusConfig&	statusConfig;
};


#endif // _STATUSHANDLER_H