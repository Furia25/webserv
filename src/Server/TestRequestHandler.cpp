/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TestRequestHandler.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/10 17:09:33 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/27 19:22:36 by vdurand          ###   ########.fr       */
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
	size_t			dataSize;

	id = connection.getClientID();
	Request &req = ongoingRequests[id];
	dataSize = connection.getReadBufferSize();
	if (dataSize > 0)
	{
		try
		{
			req.feed(connection.getReadBufferPtr(), connection.getReadBufferSize());
			if (req.isHeaderParsed() && !req.isValidated())
			{
				req.check();
				req.setValidateStatus();
			}
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
