/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/24 16:18:13 by antbonin          #+#    #+#             */
/*   Updated: 2026/06/06 20:39:01 by vdurand          ###   ########.fr       */
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
#include "HTTP/HTTPTypes.hpp"
#include "HTTP/HTTPHandler.hpp"
#include "Utils/FileSystem.hpp"

#include <cstdlib>

int main(int argc, char **argv)
{
	try
	{
		if (::signal(SIGINT, SIG_IGN) == SIG_ERR)
			throw std::runtime_error("Couldn't set handler for signal");
		if (::signal(SIGPIPE, SIG_IGN) == SIG_ERR)
			throw std::runtime_error("Couldn't set handler for signal");

		if (argc != 2)
			throw std::runtime_error("usage ./" SERV_NAME " [config.toml] -> Specified config in TOML format needed, see documentation");

		std::string			config_path = argv[1];

		if (!FileSystem::exists(config_path) || FileSystem::isDirectory(config_path))
			throw std::runtime_error("usage ./" SERV_NAME " [config.toml] -> Must be a valid file");

		Config::AppConfig	config(argv[1]);
		TCPServer			server(config.engineConfig);
		HTTPHandler			handler(config);

		if (config.loggingConfig.log_file == "")
			Logger::setDefaultStream(std::cout);
		else
			Logger::setLogFile(config.loggingConfig.log_file);

		Logger::setGlobalLevel(config.loggingConfig.log_level);
		Logger::setTickInterval(config.loggingConfig.tick_interval);
		Logger::setTickCallback(&TCPServer::tickCallback, &server);

		for (size_t i = 0; i < config.servers.size(); ++i)
		{
			const Config::ServerConfig&	server_config = *config.servers[i];
			std::stringstream			ss;
			ss << "Virtual Server \"" << server_config.name << "\" initialized with:\n";
			for (RadixTree<Config::RouteConfig *>::const_iterator it = server_config.routes.begin(); it != server_config.routes.end(); ++it)
				ss << "	Route: " << it->second->path << '\n';
			Logger::INFO() << ss.str();
			for (size_t y = 0; y < server_config.bindings.size(); ++y)
			{
				const std::pair<std::string, port_t>& binding = server_config.bindings[y];
				server.openListener(binding.first, binding.second);
			}
		}
		server.bindHandler(handler);
		try { server.run(); }
		catch (const std::exception& e)
		{
			Logger::FATAL() << "Fatal error during execution: " << e.what();
			return EXIT_FAILURE;
		}
		catch (const ForkException& e)
		{
			return EXIT_FAILURE;
		}
	}
	catch (const std::exception& e)
	{
		Logger::FATAL() << "Couldn't launch server: " << e.what();
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
