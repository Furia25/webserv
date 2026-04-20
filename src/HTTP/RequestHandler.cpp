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

RequestHandler::RequestHandler(const Config& config) : Config(config)
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
			std::cerr << e.what() << '\n';
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
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
		const ServerConfig host = serverConfig[0];
		if (serverConfig.serversConfig.search(request_host, host) != true)
		{
			Response::buildErrorResponse(connection, HTTPCode::NOT_FOUND);
			ongoingRequests.erase(id);
			return ;
		}

		const RouteConfig *route;
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
				StaticHandler(final_request, connection, physical_path, route);
			case HandlerType::CGI:
				CGIHandler(final_request, connection, physical_path, route);
			default:
				Response::buildErrorResponse(connection, HTTPCode::INTERNAL_SERVER_ERROR);
		}
		connection.setDeletable();
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
	(void)connection;
}

void RequestHandler::onError(Connection &connection)
{
	(void)connection;
}
