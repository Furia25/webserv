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
# include "HTTP/HTTPTypes.hpp"
# include "HTTP/HTTPHandler.hpp"
# include "HTTP/Response.hpp"
# include "Utils/FileSystem.hpp"
# include "Utils/IntegerUtils.hpp"

HTTPHandler::HTTPHandler(const Config::AppConfig &config)
	: clientPool(256), handlerPool(), config(config), totalRequests(0)
{
}

HTTPHandler::~HTTPHandler()
{
	for (HashMap<size_t, ClientData *>::iterator it = this->clientsData.begin();
			it != this->clientsData.end(); ++it)
	{
		if (it->second->actualHandler != NULL)
			it->second->actualHandler->~HandlerSlot();
		it->second->~ClientData();
	}
}

void	HTTPHandler::dispatchError(Connection& connection, HTTPCode code)
{
	HashMap<size_t, ClientData *>::iterator it = clientsData.find(connection.getClientID());
	
	if (it == clientsData.end())
	{
		Response response(connection, code);
			response.sendKeepAlive(false)
			.sendContentLength(0)
			.sendEnd();
		return ;
	}

	ClientData& client = *it->second;
	if (!client.routeRes.host)
		client.routeRes.host = &Router::findDefaultServer(connection.getOriginPort(), this->config);
	dispatchError(client, connection, client.request, client.body, client.routeRes, code);
}

void	HTTPHandler::dispatchError(ClientData& client, Connection& connection, const Request& request,
			Body& body, const Router::RouteResult& route_result, HTTPCode error_code)
{
	if (client.actualHandler != NULL)
	{
		this->handlerPool.release(client.actualHandler);
		client.actualHandler = NULL;
	}
	AHandler *handler = this->createHandler<ErrorHandler>(connection, request, body, route_result, error_code);
	connection.setJob(handler);
}

void HTTPHandler::launchHandler(Connection &connection, ClientData &client)
{
	if (!client.builder.get_header_parsed())
	{
		Logger::ERROR() << "No request to create the job At HTTPHandler.";
		return;
	}

	if (client.actualHandler != NULL)
	{
		this->handlerPool.release(client.actualHandler);
		client.actualHandler = NULL;
	}

	AHandler	*handler;
	this->totalRequests++;

	switch (client.routeRes.route->handler)
	{
	case HandlerType::STATIC :
		handler = this->createHandler<StaticHandler>(connection, client.request, client.body, client.routeRes, HTTPCode::OK);
		break;
	case HandlerType::REDIRECT :
		handler = this->createHandler<RedirectHandler>(connection, client.request, client.body, client.routeRes, HTTPCode::OK);
		break;
	case HandlerType::STATUS :
		handler = this->createHandler<StatusHandler>(connection, client.request, client.body, client.routeRes, HTTPCode::OK);
		break;
	case HandlerType::CGI :
		handler = NULL ;//this->createHandler<CGIHandler>(connection, client.request, client.body, client.routeRes, HTTPCode::OK);
		break;
	case HandlerType::UPLOAD :
		handler = this->createHandler<UploadHandler>(connection, client.request, client.body, client.routeRes, HTTPCode::OK);
		break;
	}
	connection.setJob(handler);
}

void	HTTPHandler::checkCompletion(Connection& connection, ClientData &client) 
{
	if (!client.routeRes.route)
		return;
	size_t bodyLength = 0;
	if (client.body.getIsStreaming()) 
	{
		if (!client.body.getFileWriter())
			return;
		bodyLength = client.body.getFileWriter()->getBytesWritten();
	}
	else 
		bodyLength = client.body.getSize();
	size_t limit = client.routeRes.route->max_body_size;
	if (limit > 0 && bodyLength > limit)
	{
		Logger::ERROR() << "Payload too large: " << bodyLength << " bytes (limit: " << limit << ")";
		dispatchError(connection, HTTPCode::PAYLOAD_TOO_LARGE);
		return;
	}
	bool is_request_finished = false;
	if (client.request.is_chunked)
	{
		if (client.chunkState == CHUNK_COMPLETE)
			is_request_finished = true;
	}
	else
	{
		size_t requestLength = client.request.content_length;
		if (bodyLength > requestLength)
		{
			dispatchError(connection, HTTPCode::PAYLOAD_TOO_LARGE);
			return;
		}
		if (bodyLength == requestLength) 
			is_request_finished = true;
	}
	if (is_request_finished) 
	{
		if (bodyLength == 0 && client.routeRes.route->handler == HandlerType::UPLOAD)
		{
			if (client.body.getIsStreaming() && client.body.getFileWriter())
			{
				std::string path = client.body.getFileWriter()->getFilePath();
				client.body.getFileWriter()->close();
				FileSystem::removeFile(path);
			}
			Logger::ERROR() << "Upload Failed 0 bytes sent";
			dispatchError(connection, HTTPCode::BAD_REQUEST);
		}
		client.body.finish();
		this->launchHandler(connection, client);
	}
}

void	HTTPHandler::receiveBodyChunk(ClientData& client, const uint8_t* fragment, size_t size)
{
	size_t toProcess = size;

	if (!client.request.is_chunked)
	{
		size_t remaining = client.request.content_length - client.body.getSize();
		toProcess = (size < remaining) ? size : remaining;
	}

	if (toProcess > 0)
		client.body.feed(fragment, toProcess);
}

bool	HTTPHandler::initializeBodyReception(Connection& connection, ClientData& client)
{
	if (!client.routeRes.route)
		return false;
	bool isStreaming = !client.body.isLessThanOneMO();
	std::string path = "";

	if (client.routeRes.route->handler == HandlerType::UPLOAD) 
	{
		const Config::UploadConfig& uploadConfig = static_cast<const Config::UploadConfig&>(*client.routeRes.route);
		std::string	fileName = client.request.path;
		size_t		pos = fileName.find_last_of('/');

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
		pathBuilder << TEMP_FILE_PATH << connection.getHash();
		path = pathBuilder.str();
	}
	client.body.init(client.request.content_length, path, isStreaming);
	return true;
}

bool	HTTPHandler::processHeaders(Connection& connection, ClientData& client, const uint8_t* fragment, size_t size)
{
	try { client.builder.feed(fragment, size); }
	catch (const std::overflow_error& e)
	{
		Logger::ERROR() << "Header DoS Attempt blocked: " << e.what();
		dispatchError(connection, HTTPCode::HEADER_FIELDS_TOO_LARGE); 
		return false;
	}

	if (!client.builder.get_header_parsed())
		return false;

	try
	{
		client.builder.print();
		client.builder.check();
	}
	catch (const std::exception& e)
	{
		dispatchError(connection, HTTPCode::BAD_REQUEST);
		return false;
	}

	client.request = client.builder.build();

	try
	{
		client.routeRes = Router::resolve(connection, this->config, client.request);
	}
	catch (const std::exception& e)
	{
		Logger::ERROR() << "Router Crashed: " << e.what();
		dispatchError(connection, HTTPCode::INTERNAL_SERVER_ERROR);
		return false;
	}
	if (client.request.method == Method::UNKNOWN)
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
					if (client.neededBytes > 10485760)
						throw HTTPException(HTTPCode::PAYLOAD_TOO_LARGE);
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
				client.body.setIsFinished(true);
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

	ClientData&		client = *this->clientsData.at(id);
	const uint8_t	*fragment = connection.getReadBufferPtr();
	size_t			dataSize = connection.getReadBufferSize();

	/*C'est juste un test pour reset le client quand on reçois des donées alors qu'on a deja parser c'est a changé*/
	if (client.actualHandler != NULL || client.builder.getCompleteStatus())
	{
		this->resetClient(client);
		connection.setJob(NULL);
	}

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
			const HashMap<std::string, std::string>& headers = client.request.getHeaders();
			if (headers.contain("expect") && headers.at("expect").find("100-continue") != std::string::npos) 
				Response(connection, HTTPCode::CONTINUE).sendEnd();
			std::vector<uint8_t> extra = client.builder.getExtraData();
			if (!extra.empty()) 
			{
				if (client.request.is_chunked)
					processChunkedData(connection, client, extra.data(), extra.size());
				else
					receiveBodyChunk(client, extra.data(), extra.size());
			}
			connection.consumeReadData(dataSize);
			checkCompletion(connection, client);
			return;
		}
		if (client.request.is_chunked)
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
		try 
		{
			dispatchError(connection, HTTPCode::INTERNAL_SERVER_ERROR);
		} catch (...) 
		{
			Response(connection, HTTPCode::INTERNAL_SERVER_ERROR)
				.sendKeepAlive(false)
				.sendContentLength(0)
				.sendEnd();
		}
	}
}

void HTTPHandler::onConnection(Connection& connection)
{
	ClientData *new_client = new (this->clientPool.acquire()) ClientData();
	this->clientsData.insert(connection.getClientID(), new_client);
}

void HTTPHandler::onDisconnection(Connection& connection)
{
	HashMap<size_t, ClientData*>::iterator it = this->clientsData.find(connection.getClientID());

	if	(it != this->clientsData.end())
	{
		ClientData *client = it->second;
		if (client->actualHandler != NULL)
			this->handlerPool.release(client->actualHandler);
		this->clientPool.release(client);
		this->clientsData.erase(it);
	}
}

void HTTPHandler::onError(Connection& connection, uint32_t error_event)
{
	if (!(error_event & EPOLLRDHUP || error_event & EPOLLHUP))
		Logger::ERROR() << "Connection:" << connection << " errored";
}

void HTTPHandler::resetClient(ClientData &client)
{
	if (client.actualHandler != NULL)
	{
		this->handlerPool.release(client.actualHandler);
		client.actualHandler = NULL;
	}
	client.builder.reset();
	client.body.reset();
	client.request = Request();
	client.chunkState = CHUNK_SIZE;
	client.neededBytes = 0;
}
