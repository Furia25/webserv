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

# ifndef _HTTPHANDLER_H
# define _HTTPHANDLER_H

# include "Server/IRequestHandler.hpp"
# include "Config/ConfigDefault.hpp"
# include "Config/Config.hpp"

# include "HTTP/HTTPHandler.hpp"
# include "HTTP/RequestBuilder.hpp"
# include "HTTP/Router.hpp"
# include "HTTP/AHandler.hpp"
# include "HTTP/Handler/ErrorHandler.hpp"
# include "HTTP/Utils/FileWriter.hpp"
# include "HTTP/Router.hpp"

# include "HTTP/Handler/CGIHandler.hpp"
# include "HTTP/Handler/ErrorHandler.hpp"
# include "HTTP/Handler/RedirectHandler.hpp"
# include "HTTP/Handler/StaticHandler.hpp"
# include "HTTP/Handler/StatusHandler.hpp"
# include "HTTP/Handler/UploadHandler.hpp"

# include "Server/IJob.hpp"
# include "HTTP/Body.hpp"
# include "Utils/FreeList.hpp"

# define TEMP_FILE_NAME	SERV_NAME "_body_"

class HTTPHandler : public IRequestHandler
{
public:

	HTTPHandler(const Config::AppConfig& config);
	~HTTPHandler();

	void		onDataReceived(Connection& connection);
	void		onConnection(Connection& connection);
	void		onDisconnection(Connection& connection);
	void		onError(Connection& connection, uint32_t error_event);

	size_t		getTotalRequests(void) const;

private:
	struct HandlerSlot
	{
		union
		{
			AlignedBuffer<StaticHandler>::type		static_handler;
			AlignedBuffer<RedirectHandler>::type	redirect_handler;
			AlignedBuffer<ErrorHandler>::type		error_handler;
			AlignedBuffer<CGIHandler>::type			cgi_handler;
			AlignedBuffer<StatusHandler>::type		status_handler;
			AlignedBuffer<UploadHandler>::type		upload_handler;
		};
		AHandler	*active;
		~HandlerSlot() { if (active) active->~AHandler(); };
	};

	struct ClientData
	{
		RequestBuilder			builder;
		Request					request;
		Body					body;
		Router::RouteResult 	routeRes;
		HandlerSlot				*actualHandler;
		size_t					neededBytes;
		std::string				sizeBuffer;

		ClientData(): actualHandler(NULL), neededBytes(0) {};
		void Reset()
		{
			this->builder.reset();
			this->body.reset();
			this->request.reset();
			this->actualHandler = NULL;
			this->neededBytes = 0;
			this->sizeBuffer.clear();
		};
	};

	HashMap<size_t, ClientData *>	clientsData;
	FreeList<ClientData>			clientPool;
	FreeList<HandlerSlot>			handlerPool;
	const Config::AppConfig&		config;
	size_t							totalRequests;

	bool	initializeBodyReception(ClientData& client);
	void	receiveBodyChunk(ClientData& client, const uint8_t* fragment, size_t size);
	bool	processHeaders(Connection& connection, ClientData& client, const uint8_t* fragment, size_t size);

	void 	launchHandler(Connection &connection, ClientData &client);
	void	checkCompletion(Connection& connection, ClientData& clientData);

	template <typename T>
	AHandler	*createHandler(Connection &connection, const Request &request,
					Body &body, const Router::RouteResult &route_result, HTTPCode status_code);

	void		dispatchError(Connection& connection, HTTPCode error_code);
	void		dispatchError(ClientData& client, Connection& connection, const Request& request, Body& body, const Router::RouteResult& route_result, HTTPCode error_code);
	void		resetClient(ClientData& client);

	bool		handleHeaderPhase(Connection& connection, ClientData& client);
	void 		switchToBodyReception(Connection& connection, ClientData& client);
	void		streamBodyFragment(Connection& connection, ClientData& client);
};

template <typename T>
inline AHandler	*HTTPHandler::createHandler(Connection& connection, const Request& request,
				Body& body, const Router::RouteResult& route_result, HTTPCode status_code)
{
	ClientData&	client_data = *this->clientsData.at(connection.getClientID());
	AHandler	*handler = NULL;
	HandlerSlot	*slot = static_cast<HandlerSlot*>(this->handlerPool.acquire());

	slot->active = NULL;
	try
	{
		handler = new (slot) T(*this, connection, request, body, route_result, status_code);
		slot->active = handler;
	}
	catch (const HTTPException& e)
	{
		try
		{
			handler = new (slot) ErrorHandler(*this, connection, request, body, route_result, status_code);
			slot->active = handler;
		}
		catch (...)
		{
			this->handlerPool.releaseRaw(slot);
			throw ;
		}
	}
	catch (...)
	{
		this->handlerPool.releaseRaw(slot);
		throw;
	}

	client_data.actualHandler = slot;
	return handler;
}

#endif // _HTTPHANDLER_H
