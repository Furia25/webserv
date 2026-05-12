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
# include "Utils/IntegerUtils.hpp"

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
	this->totalRequests++;
	switch (client.routeRes.route->handler)
	{
	case HandlerType::STATIC :
		this->createJob<StaticHandler>(connection, *client.request, client.routeRes.host, client.routeRes.route, client.routeRes.physicalPath);
		break;
	case HandlerType::REDIRECT :
		this->createJob<RedirectHandler>(connection, *client.request, client.routeRes.host, client.routeRes.route, client.routeRes.physicalPath);
		break;
	case HandlerType::STATUS :
		this->createJob<StatusHandler>(connection, *client.request, client.routeRes.host, client.routeRes.route, client.routeRes.physicalPath);
		break;
	case HandlerType::CGI :
		this->createJob<CGIHandler>(connection, *client.request, client.routeRes.host, client.routeRes.route, client.routeRes.physicalPath);
	break;
	case HandlerType::UPLOAD :
		this->createJob<UploadHandler>(connection, *client.request, client.routeRes.host, client.routeRes.route, client.routeRes.physicalPath, HTTPCode::OK);
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
		size_t requestLength = client.request->.content_length;
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
		client.request->finishBody();
		this->launchJob(connection, client);
	}
}

void	HTTPHandler::receiveBodyChunk(ClientData& client, const uint8_t* fragment, size_t size)
{
	size_t toProcess = size;

	if (!client.request->isChunked())
	{
		size_t remaining = client.request->.content_length - client.request->getBodySize();
		toProcess = (size < remaining) ? size : remaining;
	}

	if (toProcess > 0)
		client.request->getBody().feed(fragment, toProcess);
}

bool	HTTPHandler::initializeBodyReception(Connection& connection, ClientData& client)
{
	if (!client.request || !client.routeRes.route)
		return false;
	bool isStreaming = !client.request->isLessThanOneMO();
	std::string path = "";

	if (client.routeRes.route->handler == HandlerType::UPLOAD) 
	{
		const Config::UploadConfig& uploadConfig = static_cast<const Config::UploadConfig&>(*client.routeRes.route);
		std::string fileName = client.request->.path;
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
	return true;
}

bool	HTTPHandler::processHeaders(Connection& connection, ClientData& client, const uint8_t* fragment, size_t size)
{
	client.builder.feed(fragment, size);

	if (!client.builder.get_header_parsed())
		return false;

	try 
	{
		client.builder.check();
	}
	catch (const std::exception& e)
	{
		Logger::ERROR() << "Header Validation Failed: " << e.what();
		dispatchError(connection, HTTPCode::BAD_REQUEST);
		return false;
	}
	client.builder.print();
	if (client.request)
		delete client.request;
	client.request = client.builder.build();
	if (!client.request)
	{
		dispatchError(connection, HTTPCode::INTERNAL_SERVER_ERROR);
		return false;
	}
	try
	{
		client.routeRes = Router::resolve(connection, this->config, *client.request);
	}
	catch (const std::exception& e)
	{
		Logger::ERROR() << "Router Crashed: " << e.what();
		dispatchError(connection, HTTPCode::INTERNAL_SERVER_ERROR);
		return false;
	}
	if (client.request->.method == Method::UNKNOWN)
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
				size_t remainingInFragment = size - i;
				size_t toWrite = (remainingInFragment < client.neededBytes) ? remainingInFragment : client.neededBytes;
				if (toWrite > 0) 
				{
					receiveBodyChunk(client, fragment + i, toWrite);
					i += toWrite;
					client.neededBytes -= toWrite;
				}
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
				client.request->getBody().setIsFinished(true);
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
	HashMap<size_t, ClientData>::iterator it = clientsData.find(id);
	if (it == clientsData.end()) return;
		ClientData &client = it->second;
	const uint8_t *fragment = connection.getReadBufferPtr();
	size_t dataSize = connection.getReadBufferSize();
	if (dataSize == 0) return;
	try 
	{
		if (!client.builder.get_header_parsed()) 
		{
			if (!processHeaders(connection, client, fragment, dataSize)) 
			{
				connection.consumeReadData(dataSize);
				return;
			}
			initializeBodyReception(connection, client);
			if (client.request->getHeaders().contain("expect")) 
			{
				if (client.request->getHeaders().at("expect").find("100-continue") != std::string::npos)
					connection.sendData("HTTP/1.1 100 Continue\r\n\r\n");
			}
			std::vector<uint8_t> extra = client.builder.getExtraData();
			if (!extra.empty()) 
			{
				if (client.request->isChunked())
					processChunkedData(connection, client, extra.data(), extra.size());
				else
					receiveBodyChunk(client, extra.data(), extra.size());
			}
			connection.consumeReadData(dataSize);
			checkCompletion(connection, client);
			return;
		}
		if (client.request->isChunked())
			processChunkedData(connection, client, fragment, dataSize);
		else
			receiveBodyChunk(client, fragment, dataSize);
		checkCompletion(connection, client);
		connection.consumeReadData(dataSize);
	}
	catch (const std::exception &e) 
	{
		Logger::ERROR() << "Exception in onDataReceived: " << e.what();
		connection.setClosing();
		static bool is_handling_error = false;
		if (is_handling_error) 
			return; 
		is_handling_error = true;
		try 
		{
			dispatchError(connection, HTTPCode::INTERNAL_SERVER_ERROR);
		} catch (...) 
		{
			connection.sendData("HTTP/1.1 500 Internal Server Error\r\nConnection: close\r\n\r\n");
		}
		is_handling_error = false;
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
