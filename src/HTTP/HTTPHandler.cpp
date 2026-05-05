/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPHandler.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/10 17:09:33 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/20 13:59:49 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Config/Config.hpp"
# include "HTTP/Handler/CGIHandler.hpp"
# include "HTTP/Handler/ErrorHandler.hpp"
# include "HTTP/Handler/RedirectHandler.hpp"
# include "HTTP/Handler/StaticHandler.hpp"
# include "HTTP/Handler/StatusHandler.hpp"
# include "HTTP/Handler/UploadHandler.hpp"
# include "HTTP/HttpTypes.hpp"
# include "HTTP/HTTPHandler.hpp"
# include "HTTP/Response.hpp"
# include "Utils/FileSystem.hpp"
# include "Utils/Itoa.hpp"

HTTPHandler::HTTPHandler(const Config::AppConfig &config)
	: config(config)
{
}

HTTPHandler::~HTTPHandler()
{
	for (HashMap<size_t,
			ClientData>::iterator it = clientsData.begin(); it != clientsData.end(); ++it)
	{
		if (it->second.actual_job != NULL)
			delete it->second.actual_job;
	}
}

void HTTPHandler::dispatchError(Connection &connection, HTTPCode code)
{
	Response::buildErrorResponse(connection, code);
}

void HTTPHandler::dispatchError(Connection &connection,
		const Request &request, const Config::ServerConfig &host_config,
		const Config::RouteConfig &route_config, HTTPCode error_code)
{
	this->createJob<ErrorHandler>(connection, request, host_config,
			route_config, "", error_code);
}

void HTTPHandler::launchJob(Connection& connection, ClientData& client)
{
	if (!client.request)
	{
		Logger::ERROR() << "No request to create the job At HTTPHandler.";
		return;
	}
	std::cout << HandlerType::toString(client.routeRes.route->handler) << std::endl;
	switch (client.routeRes.route->handler)
	{
		case HandlerType::STATIC :
			this->createJob<StaticHandler>(connection, *client.request, *client.routeRes.host, *client.routeRes.route, client.routeRes.physicalPath);
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
			this->createJobUpload<UploadHandler>(connection, *client.request, *client.routeRes.host, *client.routeRes.route, client.routeRes.physicalPath, client.isStreaming, HTTPCode::OK);
		break;
	}
}

void	HTTPHandler::checkCompletion(Connection &connection, ClientData &client) 
{
	if (!client.request)
		return;

	size_t requestLength = client.request->getContentLength();
	size_t bodyLength = 0;

	if (client.isStreaming) 
	{
		if (!client.fileWriter)
			return;
		bodyLength = client.fileWriter->getBytesWritten();
	}
	else 
		bodyLength = client.request->getBodySize();

	if (bodyLength > requestLength)
	{
		dispatchError(connection, HTTPCode::PAYLOAD_TOO_LARGE);
		return;
	}

	if (bodyLength == requestLength) 
	{
		if (client.isStreaming && client.fileWriter)
			client.fileWriter->close();

		this->launchJob(connection, client);

		client.isStreaming = false; 
	}
}
void HTTPHandler::onDataReceived(Connection &connection)
{
	size_t	id = connection.getClientID();
	size_t	dataSize = connection.getReadBufferSize();
	if (dataSize == 0) return;

	HashMap<size_t, ClientData>::iterator	it = clientsData.find(id);
	if (it == clientsData.end()) return;

	ClientData	&client = it->second;
	const uint8_t*	fragment = connection.getReadBufferPtr();

	try 
	{
		if (!client.isStreaming && !client.builder.isHeaderParsed())
		{
            client.builder.feed(fragment, dataSize);

			if (client.builder.isHeaderParsed())
			{
				client.builder.print();
				client.request = new Request(client.builder.build());
				client.routeRes = Router::resolve(this->config, *client.request);

				if (!client.routeRes.success)
				{
					dispatchError(connection, client.routeRes.errorCode);
					return;
				}

				if (client.request->getContentLength() > 0)
				{
					std::vector<uint8_t> extra = client.builder.getExtraData();

					if (!client.request->isLessThanOneMO())
					{
						client.isStreaming = true;

						if (!client.fileWriter) client.fileWriter = new FileWriter();
						{
							std::stringstream ss;
							ss << _temp_file_path_ << id;
							client.fileWriter->open(ss.str());
						}

						if (!extra.empty())
							client.fileWriter->writeChunk(extra.data(), extra.size());
					}
					else 
					{
						client.isStreaming = false;
						if (!extra.empty())
						{
							client.request->reserveBody(client.request->getContentLength());
							client.request->appendToBody(extra.data(), extra.size());
						}
					}
					checkCompletion(connection, client);
				}
				else
					this->launchJob(connection, client);
			}
		} 
		else
		{

			if (client.isStreaming)
				client.fileWriter->writeChunk(fragment, dataSize);
			else
				client.request->appendToBody(fragment, dataSize);

			checkCompletion(connection, client);
		}
		connection.consumeReadData(dataSize);
	}
	catch (const std::exception &e)
	{
		Logger::ERROR() << "Exception: " << e.what() << '\n';
		dispatchError(connection, HTTPCode::INTERNAL_SERVER_ERROR);
	}
}

void HTTPHandler::onConnection(Connection &connection)
{
	this->clientsData.insert(connection.getClientID(), ClientData());
}

void HTTPHandler::onDisconnection(Connection &connection)
{
	HashMap<size_t, ClientData>::iterator it = this->clientsData.find(connection.getClientID());

	if	(it != this->clientsData.end())
	{
		ClientData &data = it->second;
		if	(data.request != NULL)
		{
			delete data.request;
			data.request = NULL;
		}
		if	(data.fileWriter != NULL)
		{
			delete data.fileWriter;
			data.fileWriter = NULL;
		}
		if	(data.actual_job != NULL)
		{
			delete data.actual_job;
			data.actual_job = NULL;
		}
		this->clientsData.erase(it);
	}
}

void HTTPHandler::onError(Connection &connection)
{
	Logger::ERROR() << "Connection:" << connection << " errored";
}
