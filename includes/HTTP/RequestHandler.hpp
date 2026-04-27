/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/10 17:08:53 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/20 14:00:48 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# ifndef _RequestHandler_H
# define _RequestHandler_H

# include "Server/IRequestHandler.hpp"
# include "Config/Config.hpp"
# include "HTTP/RequestBuilder.hpp"
# include "HTTP/Router.hpp"

class RequestHandler : public IRequestHandler
{
public:
	RequestHandler(const Config::AppConfig& config);
	~RequestHandler();
	void		onDataReceived(Connection& connection);
	void		onConnection(Connection& connection);
	void		onDisconnection(Connection& connection);
	void		onError(Connection& connection);
protected:
private:

	struct ClientData
	{
		RequestBuilder	builder;
		IJob			*actual_job;
	};

	template <typename T>
	void	createJob(Connection& connection, const Request& request,
				const Config::ServerConfig& host_config, const Config::RouteConfig& route_config,
				const std::string& physical_path, HTTPCode status_code = HTTPCode::OK);

	void	dispatchError(Connection& connection, HTTPCode code);
	void	dispatchError(Connection& connection, const Request& request,
				const Config::ServerConfig& host_config, const Config::RouteConfig& route_config, HTTPCode error_code);
	void	handleStaticRoute(Connection& connection, const Request& final_request,
				const Config::RouteConfig* route, const Config::ServerConfig* host, std::string physical_path);

	HashMap<size_t, ClientData>	clientsData;
	const Config::AppConfig&	config;
};

template <typename T>
inline void RequestHandler::createJob(Connection& connection, const Request& request,
	const Config::ServerConfig& host_config, const Config::RouteConfig& route_config,
	const std::string& physical_path, HTTPCode status_code)
{
	AHandler	*handler = NULL;
	try {
		handler = new T(connection, request, host_config, route_config, physical_path, status_code);
		handler->onCreation();
	}
	catch (const HTTPException& e)
	{
		handler = new ErrorHandler(connection, request, host_config, route_config, physical_path, e.getStatusCode());
	}
	connection.setJob(handler);
	this->clientsData.at(connection.getClientID())->second.actual_job = handler;
}

#endif // _RequestHandler_H
