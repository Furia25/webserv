/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TestRequestHandler.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/10 17:09:33 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/20 13:32:46 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/TestRequestHandler.hpp"
# include "Config.hpp"

TestRequestHandler::TestRequestHandler()
{
}

TestRequestHandler::~TestRequestHandler()
{
}

void TestRequestHandler::onDataReceived(Connection &connection)
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
		RouteConfig config = router.findRoute()
		ongoingRequests.erase(id);
	}
}

void TestRequestHandler::onConnection(Connection &connection)
{
	(void)connection;
}

void TestRequestHandler::onDisconnection(Connection &connection)
{
	(void)connection;
}

void TestRequestHandler::onError(Connection &connection)
{
	(void)connection;
}
