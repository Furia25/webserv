/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TestRequestHandler.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/10 17:09:33 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/16 18:06:17 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server/TestRequestHandler.hpp"

TestRequestHandler::TestRequestHandler()
{
}

TestRequestHandler::~TestRequestHandler()
{
}

void TestRequestHandler::onDataReceived(Connection &connection)
{
	int				id;
	const uint8_t	*dataPtr = connection.getReadBufferPtr();
	size_t			dataSize;

	id = connection.getClientID();
	Request &req = ongoingRequests[id];
	dataSize = connection.getReadBufferSize();
	if (dataSize > 0)
	{
		std::vector<uint8_t> buffer(dataPtr, dataPtr+ dataSize);
		try
		{
			req.feed(buffer);
			// connection.sendData();
		}
		catch(const std::exception& e)
		{
			std::cerr << e.what() << '\n';
		}
		connection.consumeReadData(dataSize);
	}
	if (req.getCompleteStatus())
	{
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
