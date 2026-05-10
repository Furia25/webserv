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
		it->second.reset();
}

void	HTTPHandler::dispatchError(Connection& connection, HTTPCode code)
{
	HashMap<size_t, ClientData>::iterator it = clientsData.find(connection.getClientID());
	ClientData* client = (it != clientsData.end()) ? &it->second : NULL;
	
	if (client && !client->request)
		client->request = new Request();
	
	Request dummyReq;
	const Request& reqRef = (client && client->request) ? *client->request : dummyReq;

	dispatchError(connection, reqRef, &Router::findDefaultServer(connection.getOriginPort(), this->config), NULL, code);
}

void	HTTPHandler::dispatchError(Connection& connection,
		const Request &request, const Config::ServerConfig *host_config,
		const Config::RouteConfig *route_config, HTTPCode error_code)
{
	this->createJob<ErrorHandler>(connection, request, host_config,
			route_config, "", error_code);
}

void	HTTPHandler::launchJob(Connection& connection, ClientData& client)
{
	if (!client.request)
	{
		Logger::ERROR() << "No request to create the job At HTTPHandler.";
		return;
	}
	switch (client.routeRes.route->handler)
	{
		case HandlerType::STATIC :
		{
			this->totalRequests++;
			this->createJob<StaticHandler>(connection, *client.request, client.routeRes.host, client.routeRes.route, client.routeRes.physicalPath);
		}
		break;
		case HandlerType::REDIRECT :
		{
			this->totalRequests++;
			// this->createJob<RedirectHandler>(connection, final_request, *res.host, *res.route, res.physicalPath);
		}
		break;
		case HandlerType::STATUS :
		{
			this->totalRequests++;
			this->createJob<StatusHandler>(connection, *client.request, client.routeRes.host, client.routeRes.route, client.routeRes.physicalPath);
		}
		break;
		case HandlerType::CGI :
		{
			this->totalRequests++;
			// this->createJob<CGIHandler>(connection, final_request, *res.host, *res.route, res.physicalPath);
		}
		break;
		case HandlerType::UPLOAD :
		{
			this->totalRequests++;
			this->createJob<UploadHandler>(connection, *client.request, client.routeRes.host, client.routeRes.route, client.routeRes.physicalPath, HTTPCode::OK);
		}
		break;
	}
}

void	HTTPHandler::ClientData::reset()
{
	builder.reset();
	if (actualJob) 
	{
		delete actualJob;
		actualJob = NULL;
	}
	if (request) 
	{
		delete request;
		request = NULL;
	}
}

void	HTTPHandler::checkCompletion(Connection& connection, ClientData &client) 
{
	if (!client.request || !client.routeRes.route)
		return;
	size_t bodyLength = 0;
	if (client.request->getBody().getIsStreaming()) 
	{
		if (!client.request->getBody().getFileWriter())
			return;
		bodyLength = client.request->getBody().getFileWriter()->getBytesWritten();
	}
	else 
		bodyLength = client.request->getBodySize();
	size_t limit = CONFIG_BODY_SIZE;
	if (limit > 0 && bodyLength > limit)
	{
		Logger::ERROR() << "Payload too large: " << bodyLength << " bytes (limit: " << limit << ")";
		dispatchError(connection, HTTPCode::PAYLOAD_TOO_LARGE);
		return;
	}
	bool isRequestFinished = false;
	if (client.request->isChunked())
	{
		if (client.chunkState == CHUNK_COMPLETE)
			isRequestFinished = true;
	}
	else
	{
		size_t requestLength = client.request->getContentLength();
		if (bodyLength > requestLength)
		{
			dispatchError(connection, HTTPCode::PAYLOAD_TOO_LARGE);
			return;
		}
		if (bodyLength == requestLength) 
			isRequestFinished = true;
	}
	if (isRequestFinished) 
	{
		if (client.request->getBody().getIsStreaming() && client.request->getBody().getFileWriter())
			client.request->getBody().getFileWriter()->close();
		this->launchJob(connection, client);
		client.request->getBody().setIsStreaming(false); 
	}
}

void	HTTPHandler::receiveBodyChunk(ClientData& client, const uint8_t* fragment, size_t size)
{
	if (!fragment || size == 0) 
		return;

	size_t remaining = client.request->getContentLength() - client.request->getBodySize();
    size_t toWrite = (size < remaining) ? size : remaining;

	if (client.request->getBody().getIsStreaming())
		client.request->getBody().getFileWriter()->writeChunk(fragment, toWrite);
	else
		client.request->getBody().feed(fragment, toWrite);
}

bool	HTTPHandler::initializeBodyReception(Connection& connection, ClientData& client)
{
	std::vector<uint8_t>	extra = client.builder.getExtraData();

	bool	isStreaming = !client.request->isLessThanOneMO();
	std::string	path = "";

	if (client.routeRes.route->handler == HandlerType::UPLOAD) 
	{
		const Config::UploadConfig& uploadConfig = static_cast<const Config::UploadConfig&>(*client.routeRes.route);
		std::string fileName = client.request->getPath();
		size_t pos = fileName.find_last_of('/');
		if (pos != std::string::npos)
			fileName = fileName.substr(pos + 1);
		if (fileName.empty())
			fileName = "uploaded_file";
		std::stringstream pathBuilder;
		pathBuilder << uploadConfig.upload_store << "/" << fileName << "_" << connection.getHash();
		path = pathBuilder.str();
	}
	else if (isStreaming) 
	{
		std::stringstream pathBuilder;
		pathBuilder << _temp_file_path_ << connection.getHash();
		path = pathBuilder.str();
	}
	client.request->initBody(path, isStreaming);
	if (!extra.empty()) 
	{
		size_t totalExpected = client.request->getContentLength();
		size_t toProcess = (extra.size() < totalExpected) ? extra.size() : totalExpected;
		if (toProcess > 0)
			receiveBodyChunk(client, extra.data(), toProcess);
	}
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

void	HTTPHandler::processChunkedData(Connection& connection, ClientData& client, const uint8_t* fragment, size_t size)
{
	size_t i = 0;
	while (i < size)
	{
		switch (client.chunkState)
		{
			case CHUNK_SIZE:
			{
				char c = fragment[i++];
				if (c == '\r')
					continue;
				if (c == '\n')
				{
					client.neededBytes = std::strtoul(client.sizeBuffer.c_str(), NULL, 16);
					client.sizeBuffer.clear();
					if (client.neededBytes == 0)
						client.chunkState = CHUNK_COMPLETE;
					else
						client.chunkState = CHUNK_DATA;
				}
				else 
					client.sizeBuffer += c;
				break;
			}
			case CHUNK_DATA:
			{
				size_t	remainingInFragment = size - i;
				size_t toWrite = std::min(remainingInFragment, client.neededBytes);
				receiveBodyChunk(client, fragment + i, toWrite);
				i += toWrite;
				client.neededBytes -= toWrite;
				if (client.neededBytes == 0)
					client.chunkState = CHUNK_TRAILER;
				break;
			}
			case CHUNK_TRAILER:
			{
				char c = fragment[i++];
				if ( c == '\n')
					client.chunkState = CHUNK_SIZE;
				break;
			}
			case CHUNK_COMPLETE:
			{
				if (fragment[i++] == '\n')
				{
					checkCompletion(connection, client);
					return ;
				}
				break;
			}
		}
	}
}

void	HTTPHandler::onDataReceived(Connection& connection)
{
	size_t id = connection.getClientID();
	size_t dataSize = connection.getReadBufferSize();
	const uint8_t *fragment = connection.getReadBufferPtr();

	HashMap<size_t, ClientData>::iterator it = clientsData.find(id);

	if (it == clientsData.end() || dataSize == 0)
		return;

	ClientData &client = it->second;

	try 
	{
		if (!client.builder.isHeaderParsed())
		{
			if (!processHeaders(connection, client, fragment, dataSize))
			{
				connection.consumeReadData(dataSize);
				return;
			}
			initializeBodyReception(connection, client);
			fragment = connection.getReadBufferPtr();
			dataSize = connection.getReadBufferSize();
		}

		if (client.builder.isHeaderParsed() && client.request)
		{
			if(client.request->isChunked())
			{
				Method m = client.request->getMethod();
				if (m == Method::GET || m == Method::HEAD)
					dispatchError(connection, *client.request, client.routeRes.host, client.routeRes.route, HTTPCode::BAD_REQUEST);
				processChunkedData(connection ,client, fragment, dataSize);
			}
			else
			{
				size_t totalExpected = client.request->getContentLength();
				size_t alreadyReceived = client.request->getBodySize();
				if (alreadyReceived < totalExpected)
				{
					size_t remaining = totalExpected - alreadyReceived;
					size_t toProcess = (dataSize < remaining) ? dataSize : remaining;
					if (toProcess > 0)
					{
						receiveBodyChunk(client, fragment, toProcess);
						alreadyReceived += toProcess;
					}
				}
				if (alreadyReceived >= totalExpected)
					checkCompletion(connection, client);
			}
		}
		connection.consumeReadData(dataSize);
	}
	catch (const std::exception &e)
	{
		Logger::ERROR() << "Exception in onDataReceived: " << e.what();
		dispatchError(connection, HTTPCode::INTERNAL_SERVER_ERROR);
		connection.setClosing();
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
