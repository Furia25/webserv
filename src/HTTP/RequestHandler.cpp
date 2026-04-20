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
# include "Config/Config.hpp"

RequestHandler::RequestHandler(const Config& config) : config(config)
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
		
		// TODO: 1. Trouver le ServerConfig grace a final_request.getHost()
		ServerConfig host = find(final_request.get)
		std::string
		// TODO: 2. Trouver le RouteConfig grace au RadixTree (routes) et final_request.getPath()
		// TODO: 3. Executer le bon handler selon la route
		
		ongoingRequests.erase(id);
	}
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
