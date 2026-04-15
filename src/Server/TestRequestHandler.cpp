/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TestRequestHandler.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/10 17:09:33 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/15 22:31:09 by vdurand          ###   ########.fr       */
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
	(void)connection;
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
