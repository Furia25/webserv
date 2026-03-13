/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TestRequestHandler.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/10 17:09:33 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/13 18:08:44 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server/TestRequestHandler.hpp"

TestRequestHandler::TestRequestHandler()
{
}

TestRequestHandler::~TestRequestHandler()
{
}

void	TestRequestHandler::onData(Connection &connection)
{
	(void)connection;
}

void	TestRequestHandler::onConnection(Connection& connection)
{
	(void)connection;
}

void	TestRequestHandler::onDisconnection(Connection& connection)
{
	(void)connection;
}

void	TestRequestHandler::onError(Connection& connection)
{
	(void)connection;
}
