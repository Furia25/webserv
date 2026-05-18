/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StaticHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 18:43:47 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/13 02:40:42 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _STATICHANDLER_H
# define _STATICHANDLER_H

# include <string>

# include "Config/Config.hpp"
# include "HTTP/AHandler.hpp"
# include "HTTP/Response.hpp"
# include "Server/Connection.hpp"

class StaticHandler : public AHandler
{
public:
	StaticHandler(
		HTTPHandler& handler,
		Connection& connection,
		const Request& request,
		Body& body,
		const Router::RouteResult& route_result,
		HTTPCode status_code = HTTPCode::OK)
	: AHandler(handler, connection, request, body, route_result, status_code), state(INIT),
	staticConfig(static_cast<const Config::StaticConfig&>(*route_result.route)) {};

	virtual	~StaticHandler() {};
	void	onExecute();
	void	onCreation();
private:

	enum State
	{
		INIT,
		SEND_HEADERS,
		SEND_BODY,
		SEND_AUTOINDEX,
		FINISHED
	};
	FileReader					fileReader;
	State						state;
	const Config::StaticConfig&	staticConfig;

	void	handleAutoindex();
	void	handleDelete();
};

#endif // _STATICHANDLER_H
