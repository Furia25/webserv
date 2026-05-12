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
# include "HTTP/RequestFactory.hpp"
# include "HTTP/Router.hpp"
# include "HTTP/AHandler.hpp"
# include "HTTP/Handler/ErrorHandler.hpp"
# include "Utils/FileWriter.hpp"
# include "HTTP/Router.hpp"
# include "Server/IJob.hpp"
# include "HTTP/Body.hpp"

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

	enum ChunkState
	{
		CHUNK_SIZE,
		CHUNK_DATA,
		CHUNK_TRAILER,
		CHUNK_COMPLETE
	};
	struct ClientData
	{
		RequestFactory			builder;
		Request					request;
		Body					body;
		Router::RouteResult 	routeRes;
		IJob					*actualJob;
		ChunkState				chunkState;
		size_t					neededBytes;
		std::string				sizeBuffer;
		ClientData(): actualJob(NULL), chunkState(CHUNK_SIZE), neededBytes(0){};
		~ClientData() 
		{ 
			if (actualJob) 
				delete actualJob;
		};
		ClientData(const ClientData& src);
		ClientData& operator=(const ClientData& other);
		void reset();
	};

	HashMap<size_t, ClientData>	clientsData;
	const Config::AppConfig&	config;
	size_t						totalRequests;

	void					processChunkedData(Connection& connection, ClientData& client, const uint8_t* fragment, size_t size);
	bool					processHeaders(Connection& connection, ClientData& client, const uint8_t* fragment, size_t size);
	bool					initializeBodyReception(Connection& connection, ClientData& client);
	void					receiveBodyChunk(ClientData& client, const uint8_t* fragment, size_t size);


	void 					launchJob(Connection &connection, ClientData &client);
	void					checkCompletion(Connection& connection, ClientData& clientData);

	template <typename T>
	void	createJob(Connection& connection, const Request& request, Body& body,
				const Config::ServerConfig *host_config, const Config::RouteConfig *route_config,
				const std::string& physical_path, HTTPCode status_code = HTTPCode::OK);

	void	dispatchError(Connection& connection, HTTPCode error_code);
	void	dispatchError(Connection& connection, const Request& request, Body& body,
				const Config::ServerConfig *host_config, const Config::RouteConfig *route_config, HTTPCode error_code);

};

template <typename T>
inline void HTTPHandler::createJob(Connection& connection, const Request& request, Body& body,
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
		handler = new T(*this, connection, request, body, host_config, route_config, physical_path, status_code);
		handler->onCreation();
		client_data.actualJob = handler;
	}
	catch (const HTTPException& e)
	{
		delete handler;
		handler = new ErrorHandler(*this, connection, request, body, host_config, route_config, physical_path, e.getStatusCode());
		client_data.actualJob = handler;
	}
	connection.setJob(handler);
}

#endif // _HTTPHandler_H
