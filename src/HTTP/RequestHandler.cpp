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
# include "Utils/FileSystem.hpp"

RequestHandler::RequestHandler(const Config::AppConfig& config) : serverConfig(config)
{
}

RequestHandler::~RequestHandler()
{
}

void RequestHandler::dispatchError(int id, Connection& connection, HTTPCode code, const Config::ServerConfig* host, const Request* req)
{
	if (host)
	{
		HashMap<HTTPCode, std::string>::const_iterator it = host->error_fallbacks.find(code);
		if (it != host->error_fallbacks.end())
		{
			std::string error_path = it->second;
			if (error_path.length() > 0 && error_path[0] != '/')
				error_path = host->root + "/" + error_path;
			else
				error_path = host->root + error_path;

			if (FileSystem::exists(error_path) && FileSystem::isFile(error_path) && FileSystem::isReadable(error_path))
			{
				if (req)
				{
					connection.addJob(new StaticHandler(*req, NULL, connection, error_path, host, code));
				}
				else
				{
					Request dummy_req(Method::GET, error_path, "", "HTTP/1.1", 0, HashMap<std::string, std::string>(), std::vector<uint8_t>());
					connection.addJob(new StaticHandler(dummy_req, NULL, connection, error_path, host, code));
				}
				ongoingRequests.erase(id);
				return ;
			}
		}
	}
	Response::buildErrorResponse(connection, code);
	ongoingRequests.erase(id);
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
			dispatchError(id, connection, HTTPCode::BAD_REQUEST, NULL, NULL);
		}
		catch(const std::exception& e)
		{
			std::cerr << "Exception: " << e.what() << '\n';
			dispatchError(id, connection, HTTPCode::INTERNAL_SERVER_ERROR, NULL, NULL);
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
		Config::ServerConfig host;
		if (serverConfig.serversConfig.search(request_host, host) != true)
		{
			dispatchError(id, connection, HTTPCode::NOT_FOUND, NULL, &final_request);
			return ;
		}

		Config::RouteConfig *route;
		if (host.routes.search(final_request.getPath(), route) != true )
		{
			dispatchError(id, connection, HTTPCode::NOT_FOUND, &host, &final_request);
			return ;
		}
		
		if (route->method_allowed[final_request.getMethod()] != true)
		{
			dispatchError(id, connection, HTTPCode::METHOD_NOT_ALLOWED, &host, &final_request);
			return ;
		}

		if (host.max_body_size < final_request.getContentLength())
		{
			dispatchError(id, connection, HTTPCode::PAYLOAD_TOO_LARGE, &host, &final_request);
			return ;
		}

		if (final_request.getPath().find("..") != std::string::npos)
		{
			dispatchError(id, connection, HTTPCode::FORBIDDEN, &host, &final_request);
			return ;
		}

		std::string physical_path = host.root + final_request.getPath();

		switch (route->handler)
		{
			case HandlerType::STATIC:
				connection.addJob(new StaticHandler(final_request, route, connection, physical_path, &host));
				break;
			default:
				dispatchError(id, connection, HTTPCode::INTERNAL_SERVER_ERROR, &host, &final_request);
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
