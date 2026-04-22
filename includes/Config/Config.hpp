/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/22 23:26:37 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/23 01:13:47 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _CONFIGSTRUCTS_H
# define _CONFIGSTRUCTS_H

# include <string>

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

struct ServerLimits
{
	uint64_t	max_body_size;

	void	load(toml::Variant& table, Config::Loader& loader);
};

struct RouteConfig
{
	std::string	path;
	HandlerType	handler;
	bool		method_allowed[Method::length];

	std::string	root;
	std::string	alias;

	virtual ~RouteConfig() {};

	virtual void	load(toml::Variant& table, Config::Loader& loader);
};

struct StaticConfig : public RouteConfig
{
	std::string	index;
	bool		autoindex;

	void	load(toml::Variant& table, Config::Loader& loader);
};

struct UploadConfig : public RouteConfig
{
	std::vector<MIME>	allowed_extensions;
	std::string			upload_store;
	uint64_t			max_file_size;
	bool				allow_overwrite;

	void	load(toml::Variant& table, Config::Loader& loader);
};

struct RedirectConfig : public RouteConfig
{
	std::string	redirect_location;
	HTTPCode	status;

	void	load(toml::Variant& table, Config::Loader& loader);
};

struct CGIConfig : public RouteConfig
{
	HashMap<std::string, std::string>	env;
	std::string							interpreter;
	std::string							bin;
	timestamp_ms						timeout;

	void	load(toml::Variant& table, Config::Loader& loader);
};

struct StatusConfig : public RouteConfig
{
	void	load(toml::Variant& table, Config::Loader& loader);
};

struct ServerConfig
{
	std::string		name;
	std::string		host;
	std::string		service;
	std::string		root;
	ServerLimits	limits;

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

	void	load(toml::Variant& table, Config::Loader& loader);
};

struct AppConfig
{
	AppConfig(const std::string& filename);
	EngineConfig			engineConfig;
	LoggingConfig			loggingConfig;
	RadixTree<ServerConfig>	serversConfig;

};

class Exception : public std::runtime_error
{
public:
	Exception(const std::string& msg) : std::runtime_error(std::string("Config: ") + msg) {}
};

} // namespace Config

#endif // _CONFIGSTRUCTS_H