/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/10 17:09:33 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/20 13:59:49 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/RequestHandler.hpp"
# include "HTTP/Response.hpp"
# include "Config/Config.hpp"
# include "HTTP/HttpTypes.hpp"
# include "HTTP/Handler.hpp"

RequestHandler::RequestHandler(const Config& config) : serverConfig(config)
{
}

RequestHandler::~RequestHandler()
{
}

void RequestHandler::onDataReceived(Connection &connection)
{
	int				id;
	size_t			dataSize;

	id = connection.getClientID();
	RequestBuilder &req = ongoingRequests[id];
	dataSize = connection.getReadBufferSize();
	if (dataSize > 0)
	{
		try
		{
			req.feed(connection.getReadBufferPtr(), connection.getReadBufferSize());
			if (req.isHeaderParsed() && !req.isValidated())
			{
				req.check();
				req.setValidateStatus(1);
			}
		}
		catch(const HTTPException& e)
		{
			req.setValidateStatus(0);
			std::cerr << "HTTPException: " << e.what() << '\n';
			Response::buildErrorResponse(connection, HTTPCode::BAD_REQUEST);
			ongoingRequests.erase(id);
		}
		catch(const std::exception& e)
		{
			std::cerr << "Exception: " << e.what() << '\n';
			Response::buildErrorResponse(connection, HTTPCode::INTERNAL_SERVER_ERROR);
			ongoingRequests.erase(id);
		}
		connection.consumeReadData(dataSize);
	}
	if (req.getCompleteStatus() && req.isValidated())
	{
		req.print();
		Request final_request = req.build();

		std::string request_host = "";
		HashMap<std::string, std::string>::const_iterator it = final_request.getHeaders().find("host");
		if (it != final_request.getHeaders().end())
		request_host = it->second;
		ServerConfig host;
		if (serverConfig.serversConfig.search(request_host, host) != true)
		{
			Response::buildErrorResponse(connection, HTTPCode::NOT_FOUND);
			ongoingRequests.erase(id);
			return ;
		}

		RouteConfig *route;
		if (host.routes.search(final_request.getPath(), route) != true )
		{
			Response::buildErrorResponse(connection, HTTPCode::NOT_FOUND);
			ongoingRequests.erase(id);
			return ;
		}
		
		if (route->method_allowed[final_request.getMethod()] != true)
		{
			Response::buildErrorResponse(connection, HTTPCode::METHOD_NOT_ALLOWED);
			ongoingRequests.erase(id);
			return ;
		}

		if (host.limits.max_body_size < final_request.getContentLength())
		{
			Response::buildErrorResponse(connection, HTTPCode::PAYLOAD_TOO_LARGE);
			ongoingRequests.erase(id);
			return ;
		}

		if (final_request.getPath().find("..") != std::string::npos)
		{
			Response::buildErrorResponse(connection, HTTPCode::FORBIDDEN);
			ongoingRequests.erase(id);
			return ;
		}

		std::string physical_path = host.root + final_request.getPath();

		switch (route->handler)
		{
			case HandlerType::STATIC:
				connection.addJob(new StaticHandler(final_request, route, connection, physical_path));
				break;
			default:
				Response::buildErrorResponse(connection, HTTPCode::INTERNAL_SERVER_ERROR);
				break;
		}
		ongoingRequests.erase(id);
	}
	return ;
}

void RequestHandler::onConnection(Connection &connection)
{
	(void)connection;
}

void RequestHandler::onDisconnection(Connection &connection)
{
	ongoingRequests.erase(connection.getClientID());
}

void RequestHandler::onError(Connection &connection)
{
	ongoingRequests.erase(connection.getClientID());
}
