/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/24 16:18:13 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/24 16:21:10 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <vector>
#include "Config/Config.hpp"
#include "Server/Address.hpp"
#include "Server/AddressResolver.hpp"
#include "Server/TCPServer.hpp"
#include "HeaderParam.hpp"
#include "Logger.hpp"
#include "HTTP/HttpTypes.hpp"
#include "HTTP/RequestHandler.hpp"

int main(int argc, char **argv)
{
	if (argc != 2)
	{
		std::cout << "error : usage ./webserv [config.toml]" << std::endl;
		return EXIT_FAILURE;
	}
	try
	{
		Config::AppConfig	my_config(argv[1]);
		TCPServer			server(my_config.engineConfig);
		RequestHandler		testHandler(my_config);

		Logger::setDefaultStream(std::cout);
		Logger::setTickInterval(5);
		Logger::setTickCallback(&TCPServer::tickCallback, &server);
		server.bindHandler(testHandler);
		server.openListener("localhost", "8080");
		server.run();
	}
	catch (const std::exception& e)
	{
		Logger::FATAL() << "Fatal error : \n" << e.what() << "\nerrno: " << strerror(errno);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
