/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/24 16:18:13 by antbonin          #+#    #+#             */
/*   Updated: 2026/06/12 17:33:35 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include <iostream>
# include <vector>
# include <cstdlib>

# include "Config/Config.hpp"
# include "Server/Address.hpp"
# include "Server/AddressResolver.hpp"
# include "Server/TCPServer.hpp"
# include "HeaderParam.hpp"
# include "Logger.hpp"
# include "HTTP/HTTPTypes.hpp"
# include "HTTP/HTTPHandler.hpp"
# include "Utils/FileSystem.hpp"

# define DEFAULT_CONFIG_PATH	"./config/default.toml"
# define DEFAULT_FALLBACK_CONFIG_NAME "DEFAULT"

static const char* DEFAULT_FALLBACK_CONFIG =
	"[[servers]]\n"
	"server_name = \"main\"\n"
	"root = \"www/\"\n"
	"[logging]\n"
	"level = \"DEBUG\"\n"
	"\n"
	"	[[servers.bindings]]\n"
	"	host = \"localhost\"\n"
	"	service = \"8080\"\n"
	"\n"
	"	[[servers.routes]]\n"
	"	handler = \"STATIC\"\n"
	"	path = \"/\"\n"
	"	methods = [\"GET\"]\n"
	"	index = \"index.html\"\n"
	"	autoindex = true\n";

static void	get_default_config(std::ifstream& config_file, std::istringstream& fallback,
				std::istream*& active_stream, std::string& config_path);
static void	open_listeners(TCPServer& server, std::vector<Config::ServerConfig *>& server_configs);
static void	remove_temp_directories(std::vector<Config::ServerConfig *>& servers);
static void	init_temp_directories(std::vector<Config::ServerConfig *>& servers);

int main(int argc, char **argv)
{
	bool started = false;

	try
	{
		if (::signal(SIGINT, SIG_IGN) == SIG_ERR)
			throw std::runtime_error("Couldn't set handler for signal");
		if (::signal(SIGPIPE, SIG_IGN) == SIG_ERR)
			throw std::runtime_error("Couldn't set handler for signal");

		std::ifstream		config_file;
		std::istringstream  fallback_stream;
		std::istream		*config_active_stream = NULL;
		std::string			config_path;

		if (argc == 1)
			get_default_config(config_file, fallback_stream, config_active_stream, config_path);
		else if (argc == 2)
		{
			config_path = argv[1];
			if (!FileSystem::exists(config_path) || FileSystem::isDirectory(config_path))
				throw std::runtime_error("usage ./" SERV_NAME " [config.toml] -> Must be a valid readable file");
			config_file.open(config_path.c_str());
			config_active_stream = &config_file;
		}
		else
			throw std::runtime_error("usage ./" SERV_NAME " [config.toml] -> Too many arguments");

		if (!config_active_stream || !(*config_active_stream))
			throw std::runtime_error("Couldn't open config stream");

		Config::AppConfig	config(*config_active_stream, config_path);
		TCPServer			server(config.engineConfig);
		HTTPHandler			handler(config);

		config_file.close();
		remove_temp_directories(config.servers);
		init_temp_directories(config.servers);

		if (config.loggingConfig.log_file == "")
			Logger::setDefaultStream(std::cout);
		else
			Logger::setLogFile(config.loggingConfig.log_file);

		Logger::setGlobalLevel(config.loggingConfig.log_level);
		Logger::setTickInterval(config.loggingConfig.tick_interval);
		Logger::setTickCallback(&TCPServer::tickCallback, &server);

		open_listeners(server, config.servers);
		server.bindHandler(handler);

		started = true;

		try { server.run(); }
		catch (const ForkException& e) { return EXIT_FAILURE; }
		remove_temp_directories(config.servers);
	}
	catch (const std::exception& e)
	{
		if (started)
			Logger::FATAL() << "Fatal error during execution: " << e.what();
		else
			Logger::FATAL() << "Unable to launch server: " << e.what();
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

static void	get_default_config(std::ifstream& config_file, std::istringstream& fallback,
				std::istream*& active_stream, std::string& config_path)
{
	if (FileSystem::exists(DEFAULT_CONFIG_PATH))
	{
		config_path = DEFAULT_CONFIG_PATH;
		config_file.open(DEFAULT_CONFIG_PATH);
		active_stream = &config_file;
	}
	else
	{
		config_path = DEFAULT_FALLBACK_CONFIG_NAME;
		fallback.str(DEFAULT_FALLBACK_CONFIG);
		active_stream = &fallback;
		Logger::INFO() << "No default config file found (" DEFAULT_CONFIG_PATH "), using built-in defaults";
	}
}

static void open_listeners(TCPServer& server, std::vector<Config::ServerConfig *>& server_configs)
{
	for (size_t i = 0; i < server_configs.size(); ++i)
	{
		const Config::ServerConfig&	server_config = *server_configs[i];
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
}

static void	init_temp_directories(std::vector<Config::ServerConfig *>& servers)
{
	for (size_t index = 0; index < servers.size(); index++)
	{
		Config::ServerConfig& server_config = *servers[index];
		if (FileSystem::exists(server_config.tmp_dir_path))
		{
			if (!FileSystem::isWritable(server_config.tmp_dir_path) || !FileSystem::isReadable(server_config.tmp_dir_path))
				throw std::runtime_error("No permission to write and read on temp directories: " + server_config.tmp_dir_path);
			return ;
		}

		if (::mkdir(server_config.tmp_dir_path.c_str(), 0755) == -1) 
			throw std::runtime_error(std::string("Unable to create temp directories: ") + strerror(errno));
	}
}

static void remove_temp_directories(std::vector<Config::ServerConfig *>& servers)
{
	for (size_t index = 0; index < servers.size(); index++)
	{
		Config::ServerConfig& server_config = *servers[index];
		if (!FileSystem::exists(server_config.tmp_dir_path))
			return ;
		if (FileSystem::removeDirectoryRecursive(server_config.tmp_dir_path) == -1)
			throw std::runtime_error("Unable to remove temp directories");
	}
}

