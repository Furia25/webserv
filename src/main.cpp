/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 14:24:55 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/22 13:05:47 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <vector>
#include "Config/Config.hpp"
#include "Config/ConfigBuilder.hpp"
#include "Server/Address.hpp"
#include "Server/AddressResolver.hpp"
#include "Server/TCPServer.hpp"
#include "HeaderParam.hpp"
#include "Logger.hpp"
#include "HTTP/HttpTypes.hpp"
#include "HTTP/RequestHandler.hpp"

int main(int argc, char **argv)
{
	if (argc == 2)
	{
		Config my_config;
		ConfigBuilder builder;
		builder.from_file(my_config, argv[1]);
		TCPServer			server;
		RequestHandler		testHandler(my_config);
		
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
		return 0;
	}
	std::cout << "error : usage ./webserv [config.toml]" << std::endl;
}
