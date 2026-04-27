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
# include "HTTP/StaticHandler.hpp"
# include "Utils/FileSystem.hpp"
# include "HTTP/AutoindexHandler.hpp"
# include "HTTP/DeleteHandler.hpp"
# include <dirent.h>

RequestHandler::RequestHandler(const Config::AppConfig& config): router(config)
{
}

RequestHandler::~RequestHandler()
{
}

void RequestHandler::manageJobs(Connection &connection)
{
	int id = connection.getClientID();


	if (ongoingJobs.count(id))
	{
		IJob* job = ongoingJobs[id];

		if (job->isFinished()) 
		{
			delete job;
			ongoingJobs.erase(id);
			connection.setJob(NULL);
		}
	}
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
					connection.addJob(new StaticHandler(*req, connection, error_path, host, code));
				}
				else
				{
					Request dummy_req(Method::GET, error_path, "", "HTTP/1.1", 0, HashMap<std::string, std::string>(), std::vector<uint8_t>());
					connection.addJob(new StaticHandler(dummy_req, connection, error_path, host, code));
				}
				ongoingRequests.erase(id);
				return ;
			}
		}
	}
	Response::buildErrorResponse(connection, code);
	ongoingRequests.erase(id);
}

void RequestHandler::handleStaticRoute(int id, Connection& connection, const Request& final_request, const Config::RouteConfig* route, const Config::ServerConfig* host, std::string physical_path)
{
	if (final_request.getMethod() == Method::GET || final_request.getMethod() == Method::HEAD)
	{
		if (!FileSystem::exists(physical_path))
		{
			dispatchError(id, connection, HTTPCode::NOT_FOUND, host, &final_request);
			return;
		}

		if (FileSystem::isDirectory(physical_path))
		{
			const Config::StaticConfig* static_config = static_cast<const Config::StaticConfig*>(route);
			std::string separator = (physical_path.empty() || physical_path[physical_path.length() - 1] == '/') ? "" : "/";
			std::string index_file = physical_path + separator + static_config->index;
			
			if (!static_config->index.empty() && FileSystem::exists(index_file))
			{
				physical_path = index_file;
			}
			else if (static_config->autoindex)
			{
				connection.addJob(new AutoindexHandler(final_request, connection, physical_path));
				ongoingRequests.erase(id);
				return;
			}
			else
			{
				dispatchError(id, connection, HTTPCode::FORBIDDEN, host, &final_request);
				return;
			}
		}

		if (!FileSystem::isReadable(physical_path))
		{
			dispatchError(id, connection, HTTPCode::FORBIDDEN, host, &final_request);
			return;
		}
	}
	connection.addJob(new StaticHandler(final_request, connection, physical_path, host));
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

		RouteResult res = router.resolve(final_request);

		if (!res.success) 
		{
			dispatchError(id, connection, res.errorCode, res.host, &final_request);
			return;
		}
		IJob* newJob = NULL;
		if (final_request.getMethod() == Method::DELETE)
    		newJob = new DeleteHandler(connection, res.physicalPath);
		else if (res.route->handler == HandlerType::STATIC) 
    		handleStaticRoute(id, connection, final_request, res.route, res.host, res.physicalPath);
		ongoingJobs[id] = newJob;
		connection.setJob(newJob);
		ongoingRequests.erase(id);
	}
}

void RequestHandler::onConnection(Connection &connection)
{
	(void)connection;
}

void RequestHandler::onDisconnection(Connection &connection)
{
	int id = connection.getClientID();
    ongoingRequests.erase(id);

    if (ongoingJobs.count(id)) {
        delete ongoingJobs[id];
        ongoingJobs.erase(id);
    }
	ongoingRequests.erase(connection.getClientID());
}

void RequestHandler::onError(Connection &connection)
{
	ongoingRequests.erase(connection.getClientID());
}
