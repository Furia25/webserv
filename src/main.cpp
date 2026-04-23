/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 14:24:55 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/23 03:00:21 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <vector>
#include "Server/Address.hpp"
#include "Server/AddressResolver.hpp"
#include "Server/TCPServer.hpp"
#include "HeaderParam.hpp"
#include "Logger.hpp"
#include "HTTP/HttpTypes.hpp"
#include "HTTP/TestRequestHandler.hpp"

int main()
{
	TCPServer			server;
	TestRequestHandler	testHandler;

	Logger::setDefaultStream(std::cout);
	Logger::setTickInterval(5);
	Logger::setTickCallback(&TCPServer::tickCallback, &server);
	server.bindHandler(testHandler);
	server.openListener("localhost", "8080");
	try {
		server.run();
	} catch (const std::exception& e)
	{
		Logger::FATAL() << "Unable to recover from fatal error : \"" << e.what() << "\"\n	errno: " << strerror(errno);
	}
}
