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
	: config(config), totalRequests(0)
{
}

HTTPHandler::~HTTPHandler()
{
	for (HashMap<size_t, ClientData>::iterator it = clientsData.begin(); it != clientsData.end(); ++it)
	{
		it->second.reset();
	}
}

void HTTPHandler::dispatchError(Connection& connection, HTTPCode code)
{
	dispatchError(connection, Request(), &Router::findDefaultServer(connection.getOriginPort(), this->config), NULL, code);
}

void HTTPHandler::dispatchError(Connection& connection,
		const Request &request, const Config::ServerConfig *host_config,
		const Config::RouteConfig *route_config, HTTPCode error_code)
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
	switch (client.routeRes.route->handler)
	{
		case HandlerType::STATIC :
			this->createJob<StaticHandler>(connection, *client.request, client.routeRes.host, client.routeRes.route, client.routeRes.physicalPath);
		break;
		case HandlerType::REDIRECT :
			// this->createJob<RedirectHandler>(connection, final_request, *res.host, *res.route, res.physicalPath);
		break;
		case HandlerType::STATUS :
			this->createJob<StatusHandler>(connection, *client.request, client.routeRes.host, client.routeRes.route, client.routeRes.physicalPath);
		break;
		case HandlerType::CGI :
			// this->createJob<CGIHandler>(connection, final_request, *res.host, *res.route, res.physicalPath);
		break;
		case HandlerType::UPLOAD :
			this->createJobUpload<UploadHandler>(connection, *client.request, client.routeRes.host, client.routeRes.route, client.routeRes.physicalPath, client.isStreaming, HTTPCode::OK);
		break;
	}
}

void	HTTPHandler::ClientData::reset()
{
	builder.reset();
	if (request) 
	{
		delete request;
		request = NULL;
	}
	if (fileWriter) 
	{
		if (isStreaming) 
		{
			fileWriter->close();
			std::remove(destinationPath.c_str());
		}
		delete fileWriter;
		fileWriter = NULL;
	}
	if (actualJob) 
	{
		delete actualJob;
		actualJob = NULL;
	}
	isStreaming = false;
	destinationPath = "";
}

void	HTTPHandler::checkCompletion(Connection& connection, ClientData &client) 
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

void HTTPHandler::receiveBodyChunk(ClientData& client, const uint8_t* fragment, size_t size)
{
	if (!fragment || size == 0) 
		return;

	if (client.isStreaming)
		client.fileWriter->writeChunk(fragment, size);
	else
		client.request->appendToBody(fragment, size);
}

void	HTTPHandler::setupStreamDestination(Connection& connection, ClientData& client)
{
	if (!client.fileWriter) 
		client.fileWriter = new FileWriter();

	std::stringstream pathBuilder;

	if (client.routeRes.route->handler == HandlerType::UPLOAD) 
	{
		const Config::UploadConfig& uploadConfig = static_cast<const Config::UploadConfig&>(*client.routeRes.route);
		std::string fileName = client.request->getPath();
		size_t pos = fileName.find_last_of('/');
		if (pos != std::string::npos)
			fileName = fileName.substr(pos + 1);

		pathBuilder << uploadConfig.upload_store << "/" << fileName << "_" << connection.getHash();
	} 
	else 
		pathBuilder << _temp_file_path_ << connection.getHash();

	client.destinationPath = pathBuilder.str();
	client.fileWriter->open(client.destinationPath);
}

bool	HTTPHandler::initializeBodyReception(Connection& connection, ClientData& client)
{
	std::vector<uint8_t> extra = client.builder.getExtraData();
	client.isStreaming = !client.request->isLessThanOneMO();

	if (client.isStreaming)
		setupStreamDestination(connection, client);
	else 
		client.request->reserveBody(client.request->getContentLength());

	if (!extra.empty()) 
		receiveBodyChunk(client, extra.data(), extra.size());

	checkCompletion(connection, client);
	return true;
}

bool	HTTPHandler::processHeaders(Connection& connection, ClientData& client, const uint8_t* fragment, size_t size)
{
	client.builder.feed(fragment, size);

	if (!client.builder.isHeaderParsed())
		return false;

	client.builder.print();
	client.request = new Request(client.builder.build());
	client.routeRes = Router::resolve(connection, this->config, *client.request);

	if (client.request->getMethod() == Method::UNKNOWN)
	{
		dispatchError(connection, HTTPCode::NOT_IMPLEMENTED);
		return false;
	}
	if (!client.routeRes.success)
	{
		dispatchError(connection, client.routeRes.errorCode);
		return false;
	}
	return true;
}

void	HTTPHandler::onDataReceived(Connection& connection)
{
	size_t	id = connection.getClientID();
    size_t	dataSize = connection.getReadBufferSize();

	HashMap<size_t, ClientData>::iterator	it = clientsData.find(id);
	if (it == clientsData.end() || dataSize < 1)
		return;

	ClientData	&client = it->second;

	if (client.actualJob != NULL && connection.getJob() == NULL)
		client.reset();

	const uint8_t *fragment = connection.getReadBufferPtr();

	try 
	{
		if (!client.isStreaming && !client.builder.isHeaderParsed())
		{
			if (processHeaders(connection, client, fragment, dataSize))
			{
				if (client.request->getContentLength() > 0)
					initializeBodyReception(connection, client);
				else
					this->launchJob(connection, client);
			}
		}
		else
		{
			receiveBodyChunk(client, fragment, dataSize);
			checkCompletion(connection, client);
		}
		connection.consumeReadData(dataSize);
		this->totalRequests++;
	}
	catch (const std::exception &e)
	{
		Logger::ERROR() << "Exception: " << e.what() << '\n';
		dispatchError(connection, HTTPCode::INTERNAL_SERVER_ERROR);
	}
}

void HTTPHandler::onConnection(Connection& connection)
{
	this->clientsData.insert(connection.getClientID(), ClientData());
}

void HTTPHandler::onDisconnection(Connection& connection)
{
	HashMap<size_t, ClientData>::iterator it = this->clientsData.find(connection.getClientID());

	if	(it != this->clientsData.end())
	{
		it->second.reset();
		this->clientsData.erase(it);
	}
}

void HTTPHandler::onError(Connection& connection)
{
	Logger::ERROR() << "Connection:" << connection << " errored";
}
