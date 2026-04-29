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
# include "Utils/FileSystem.hpp"
# include "HTTP/Handler/StaticHandler.hpp"
# include "HTTP/Handler/ErrorHandler.hpp"
# include "HTTP/Handler/StatusHandler.hpp"
# include "HTTP/Handler/UploadHandler.hpp"
# include "HTTP/Handler/CGIHandler.hpp"
# include "HTTP/Handler/RedirectHandler.hpp"
# include <dirent.h>

RequestHandler::RequestHandler(const Config::AppConfig& config) : config(config) {}

RequestHandler::~RequestHandler()
{
	for (HashMap<size_t, ClientData>::iterator it = clientsData.begin(); it != clientsData.end(); ++it)
	{
		if (it->second.actual_job != NULL)
			delete it->second.actual_job;
	}
}

void	RequestHandler::dispatchError(Connection& connection, HTTPCode code)
{
	Response::buildErrorResponse(connection, code);
}

void RequestHandler::dispatchError(Connection& connection, const Request& request, const Config::ServerConfig& host_config, const Config::RouteConfig& route_config, HTTPCode error_code)
{
	this->createJob<ErrorHandler>(connection, request, host_config, route_config, "", error_code);
}

void RequestHandler::onDataReceived(Connection& connection)
{
	size_t			id = connection.getClientID();
	size_t			dataSize;
	HashMap<size_t, ClientData>::iterator	iterator = clientsData.find(id);

	RequestBuilder& req = iterator->second.builder;
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
			dispatchError(connection, e.getStatusCode());
		}
		catch(const std::exception& e)
		{
			Logger::ERROR() << "Exception: " << e.what() << '\n';
			dispatchError(connection, HTTPCode::INTERNAL_SERVER_ERROR);
		}
		connection.consumeReadData(dataSize);
	}
	if (req.getCompleteStatus() && req.isValidated())
	{
		req.print();
		Request final_request = req.build();
		Router::RouteResult res = Router::resolve(this->config, final_request);

		if (!res.success)
		{
			dispatchError(connection, final_request, *res.host, *res.route, res.errorCode);
			return ;
		}
		switch (res.route->handler)
		{
		case HandlerType::STATIC :
			this->createJob<StaticHandler>(connection, final_request, *res.host, *res.route, res.physicalPath);
		break;
		case HandlerType::REDIRECT :
			// this->createJob<RedirectHandler>(connection, final_request, *res.host, *res.route, res.physicalPath);
		break;
		case HandlerType::STATUS :
			// this->createJob<StatusHandler>(connection, final_request, *res.host, *res.route, res.physicalPath);
		break;
		case HandlerType::CGI :
			// this->createJob<CGIHandler>(connection, final_request, *res.host, *res.route, res.physicalPath);
		break;
		case HandlerType::UPLOAD :
			// this->createJob<UploadHandler>(connection, final_request, *res.host, *res.route, res.physicalPath);
		break;
		}
		req.reset();
	}
}

void RequestHandler::onConnection(Connection& connection)
{
	this->clientsData.insert(connection.getClientID(), ClientData());
}

void RequestHandler::onDisconnection(Connection& connection)
{
	if (this->clientsData.contain(connection.getClientID()))
	{
		ClientData& data = this->clientsData.at(connection.getClientID());

		if (data.actual_job != NULL)
		{
			delete data.actual_job;
			data.actual_job = NULL;
		}
		this->clientsData.erase(connection.getClientID());
	}
}

void RequestHandler::onError(Connection& connection)
{
	Logger::ERROR() << "Connection:" << connection << " errored";
}

