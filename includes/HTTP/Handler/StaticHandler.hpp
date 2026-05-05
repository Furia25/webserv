/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StaticHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 18:43:47 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/05 18:22:48 by vdurand          ###   ########.fr       */
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
		Connection& connection,
		const Request& request,
		const Config::ServerConfig& host_config,
		const Config::RouteConfig& route_config,
		const std::string& physical_path,
		HTTPCode status_code = HTTPCode::OK)
	: AHandler(connection, request, host_config, route_config, physical_path, status_code), state(INIT),
	staticConfig(static_cast<const Config::StaticConfig&>(route_config)) {};

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
