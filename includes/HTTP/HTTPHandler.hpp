/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPHandler.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/10 17:08:53 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/20 14:00:48 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# ifndef _HTTPHandler_H
# define _HTTPHandler_H

# include "Config/ConfigDefault.hpp"
# include "Server/IRequestHandler.hpp"
# include "Config/Config.hpp"
# include "HTTP/HTTPHandler.hpp"
# include "HTTP/RequestBuilder.hpp"
# include "HTTP/Router.hpp"
# include "HTTP/AHandler.hpp"
# include "HTTP/Handler/ErrorHandler.hpp"
# include "Utils/FileWriter.hpp"
# include "HTTP/Router.hpp"
# include "Server/IJob.hpp"

# define _temp_file_path_ "/tmp/" SERV_NAME "_upload_"

class HTTPHandler : public IRequestHandler
{
public:
	HTTPHandler(const Config::AppConfig& config);
	~HTTPHandler();
	void		onDataReceived(Connection& connection);
	void		onConnection(Connection& connection);
	void		onDisconnection(Connection& connection);
	void		onError(Connection& connection);

	size_t		getTotalRequests(void) const { return this->totalRequests; };
protected:
private:

	struct ClientData
	{
		RequestBuilder			builder;
		Request*				request;
		FileWriter*				fileWriter;
		Router::RouteResult 	routeRes;
		bool					isStreaming;
		IJob					*actualJob;
		ClientData(): request(NULL), fileWriter(NULL), isStreaming(false), actualJob(NULL) {};
		void reset();
	};

	HashMap<size_t, ClientData>	clientsData;
	const Config::AppConfig&	config;
	size_t						totalRequests;

	void 					launchJob(Connection &connection, ClientData &client);
	void					checkCompletion(Connection& connection, ClientData& clientData);

	template <typename T>
	void	createJob(Connection& connection, const Request& request,
				const Config::ServerConfig *host_config, const Config::RouteConfig *route_config,
				const std::string& physical_path, HTTPCode status_code = HTTPCode::OK);
				
	template <typename T>
	void	createJobUpload(Connection& connection, const Request& request,
	const Config::ServerConfig *host_config, const Config::RouteConfig *route_config,
	const std::string& physical_path, bool isUpload, HTTPCode status_code);

	void	dispatchError(Connection& connection, HTTPCode error_code);
	void	dispatchError(Connection& connection, const Request& request,
				const Config::ServerConfig *host_config, const Config::RouteConfig *route_config, HTTPCode error_code);

};

template <typename T>
inline void HTTPHandler::createJob(Connection& connection, const Request& request,
	const Config::ServerConfig *host_config, const Config::RouteConfig *route_config,
	const std::string& physical_path, HTTPCode status_code)
{
	ClientData& client_data = this->clientsData.at(connection.getClientID());
	if (client_data.actualJob != NULL)
	{
		delete client_data.actualJob;
		client_data.actualJob = NULL;
	}
	AHandler	*handler = NULL;
	try {
		handler = new T(*this, connection, request, host_config, route_config, physical_path, status_code);
		handler->onCreation();
		client_data.actualJob = handler;
	}
	catch (const HTTPException& e)
	{
		delete handler;
		handler = new ErrorHandler(*this, connection, request, host_config, route_config, physical_path, e.getStatusCode());
		client_data.actualJob = handler;
	}
	connection.setJob(handler);
}

template <typename T>
inline void HTTPHandler::createJobUpload(Connection& connection, const Request& request,
	const Config::ServerConfig *host_config, const Config::RouteConfig *route_config,
	const std::string& physical_path, bool isUpload, HTTPCode status_code)
{
	ClientData& client_data = this->clientsData.at(connection.getClientID());
	if (client_data.actualJob != NULL)
	{
		delete client_data.actualJob;
		client_data.actualJob = NULL;
	}
	AHandler	*handler = NULL;
	try {
		handler = new T(*this, connection, request, host_config, route_config, physical_path, status_code, isUpload);
		handler->onCreation();
		client_data.actualJob = handler;
	}
	catch (const HTTPException& e)
	{
		delete handler;
		handler = new ErrorHandler(*this, connection, request, host_config, route_config, physical_path, e.getStatusCode());
		client_data.actualJob = handler;
	}
	connection.setJob(handler);
}

#endif // _HTTPHandler_H
