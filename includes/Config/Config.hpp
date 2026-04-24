/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/22 23:26:37 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/24 15:48:31 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _CONFIGSTRUCTS_H
# define _CONFIGSTRUCTS_H

# include <string>
# include <sstream>
# include <cstdlib>

# include "ConfigDefault.hpp"
# include "ConfigLoader.hpp"
# include "Config/toml.hpp"

# include "Utils/RadixTree.hpp"
# include "HTTP/HttpTypes.hpp"
# include "Utils/HashedTimingWheel.hpp"
# include "Utils/HashMap.hpp"
# include "Logger.hpp"

namespace Config
{

struct ServerConfig;

struct RouteConfig
{
	HandlerType	handler;
	std::string	path;
	bool		method_allowed[Method::length];
	uint64_t	max_body_size;

	std::string	root;
	std::string	alias;

	const ServerConfig	*server_config;

	RouteConfig(const ServerConfig *server_config) : server_config(server_config) {};
	virtual ~RouteConfig() {};

	virtual void		loadChild(toml::Variant& table, Config::Loader& loader) = 0;
	void				load(toml::Variant& table, Config::Loader& loader);
	void				loadAllowedMethod(toml::Variant& table, Config::Loader& loader);
};

struct StaticConfig : public RouteConfig
{
	std::string	index;
	bool		autoindex;

	StaticConfig(const ServerConfig *server_config) : RouteConfig(server_config) {};

	void	loadChild(toml::Variant& table, Config::Loader& loader);
};

struct UploadConfig : public RouteConfig
{
	std::vector<MIME>	allowed_extensions;
	std::string			upload_store;
	bool				allow_overwrite;

	UploadConfig(const ServerConfig *server_config) : RouteConfig(server_config) {};

	void	loadChild(toml::Variant& table, Config::Loader& loader);
	void	loadAllowedExtensions(toml::Variant& table, Config::Loader& loader);
};

struct RedirectConfig : public RouteConfig
{
	std::string	redirect_location;
	HTTPCode	status;

	RedirectConfig(const ServerConfig *server_config) : RouteConfig(server_config) {};

	void	loadChild(toml::Variant& table, Config::Loader& loader);
};

struct CGIConfig : public RouteConfig
{
	HashMap<std::string, std::string>	env;
	std::string							interpreter;
	std::string							bin;
	timestamp_ms						timeout;

	CGIConfig(const ServerConfig *server_config) : RouteConfig(server_config) {};

	void	loadChild(toml::Variant& table, Config::Loader& loader);
};

struct StatusConfig : public RouteConfig
{
	StatusConfig(const ServerConfig *server_config) : RouteConfig(server_config) {};

	void	loadChild(toml::Variant& table, Config::Loader& loader) {};
};

struct ServerConfig
{
	std::string		name;
	std::string		host;
	std::string		service;
	std::string		root;
	uint64_t		max_body_size;

	RadixTree<RouteConfig *>		routes;
	HashMap<HTTPCode, std::string>	error_fallbacks;

	~ServerConfig();

	void	load(toml::Variant& table, Config::Loader& loader);
	void	loadErrors(toml::Table& errors_table, Config::Loader& loader);
	void	loadRoutes(toml::Array&	routes_array, Config::Loader& loader);
};

struct EngineConfig
{
	size_t			max_events;
	timestamp_ms	max_timeout;
	timestamp_ms	closing_timeout;
	size_t			read_size;
	size_t			max_read_limit;

	void	load(toml::Variant& table, Config::Loader& loader);
};

struct LoggingConfig
{
	std::string	log_file;
	LogLevel	log_level;
	uint64_t	tick_interval;

	void	load(toml::Variant& table, Config::Loader& loader);
};

struct AppConfig
{
	AppConfig(const std::string& filename);
	EngineConfig				engineConfig;
	LoggingConfig				loggingConfig;
	RadixTree<ServerConfig *>	servers;

	~AppConfig();
};

class Exception : public std::runtime_error
{
public:
	Exception(const std::string& msg) : std::runtime_error(std::string("Config: ") + msg) {}
};

} // namespace Config

#endif // _CONFIGSTRUCTS_H