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

size_t	HTTPHandler::getTotalRequests() const
{
	return this->totalRequests;
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

	AHandler	*handler = NULL;
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
		handler = this->createHandler<CGIHandler>(connection, client.request, client.body, client.routeRes, HTTPCode::OK);
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
	if (!client.body.getFileWriter())
		return;

	bodyLength = client.body.getFileWriter()->getBytesWritten();
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
		if (client.body.hasFinished())
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
		client.body.finish();
		this->launchHandler(connection, client);
		connection.consumeReadData(connection.getReadBufferSize());
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

bool	HTTPHandler::initializeBodyReception(ClientData& client)
{
	std::string path = "";
	std::stringstream pathBuilder;
	pathBuilder << FileSystem::GenerateUniqueFilename(client.routeRes.host->tmp_dir_path + TEMP_FILE_NAME);
	path = pathBuilder.str();
	client.body.init(client.request.content_length, path, client.routeRes.route->max_body_size);
	return true;
}

bool	HTTPHandler::processHeaders(Connection& connection, ClientData& client, const uint8_t* fragment, size_t size)
{
	try { client.builder.feed(fragment, size); }
	catch (const HTTPException& e)
	{
		dispatchError(connection, e.getStatusCode());
		return true;
	}

	if (!client.builder.get_header_parsed())
		return false;

	try
	{
		// #if HTTP_DEBUG == true
		// 	client.builder.print();
		// #endif
		client.builder.check();
	}
	catch (const HTTPException& e)
    {
        dispatchError(connection, e.getStatusCode());
        return false;
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

bool	HTTPHandler::handleHeaderPhase(Connection& connection, ClientData& client)
{
	if (!processHeaders(connection, client, connection.getReadBufferPtr(), connection.getReadBufferSize())) 
	{
		connection.consumeReadData(connection.getReadBufferSize());
		return false;
	}
	return true;
}

void HTTPHandler::switchToBodyReception(Connection& connection, ClientData& client)
{
	initializeBodyReception(client);
	
	const HashMap<std::string, std::string>& headers = client.request.getHeaders();
	if (headers.contain("expect") && headers.at("expect").find("100-continue") != std::string::npos) 
		Response(connection, HTTPCode::CONTINUE).sendEnd();

	size_t header_end = client.builder.findHeaderEnd();
	if (header_end != std::string::npos)
		connection.consumeReadData(header_end + 4); 
}

void	HTTPHandler::streamBodyFragment(Connection& connection, ClientData& client)
{
	const uint8_t*	data = connection.getReadBufferPtr();
	size_t			size = connection.getReadBufferSize();
	size_t          toProcess = size;

	if (size == 0)
		return ;

	if (!client.request.is_chunked)
	{
		size_t remaining = client.request.content_length - client.body.getSize();
		toProcess = (size < remaining) ? size : remaining;
	}

	if (toProcess > 0)
	{
		if (client.request.is_chunked)
			toProcess = client.body.feedChunked(data, toProcess);
		else
			receiveBodyChunk(client, data, toProcess);
	}
	connection.consumeReadData(toProcess);
}

void	HTTPHandler::onDataReceived(Connection& connection)
{
	ClientData&		client = *this->clientsData.at(connection.getClientID());

	if (client.actualHandler != NULL)
	{
		if (client.actualHandler->active == NULL || !client.actualHandler->active->isFinished())
			return ;
		this->resetClient(client);
		connection.setJob(NULL);
	}

	try
	{
		if (!client.builder.get_header_parsed())
		{
			if (!handleHeaderPhase(connection, client)) 
				return;
			switchToBodyReception(connection, client);
			// checkCompletion(connection, client);
			// connection.consumeReadData(connection.getReadBufferSize());
			// return;
		}
		streamBodyFragment(connection, client);
		checkCompletion(connection, client);
		// connection.consumeReadData(connection.getReadBufferSize());
	}
	catch (const std::exception &e) 
	{
		Logger::ERROR() << "Exception in onDataReceived: " << e.what();
		connection.setClosing();
		try 
		{
			dispatchError(connection, HTTPCode::INTERNAL_SERVER_ERROR);
		}
		catch (...) 
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
	if (error_event & EPOLLHUP || error_event & EPOLLRDHUP)
		return ;

	int			error_code = 0;
	socklen_t	len = sizeof(error_code);
	const char	*error = "Unknown";

	if (getsockopt(connection.getSocket().getFd(), SOL_SOCKET, SO_ERROR, &error_code, &len) == 0)
		error = strerror(error_code);
	Logger::ERROR() << "Connection:" << connection << " errored " << error;
}

void HTTPHandler::resetClient(ClientData &client)
{
	if (client.actualHandler != NULL)
	{
		this->handlerPool.release(client.actualHandler);
		client.actualHandler = NULL;
	}
	client.Reset();
}
