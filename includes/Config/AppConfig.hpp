/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AppConfig.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 17:53:32 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/22 18:58:46 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _CONFIGSTRUCTS_H
# define _CONFIGSTRUCTS_H

# include <string>

# include "ConfigDefault.hpp"
# include "Config/toml.hpp"

# include "Utils/RadixTree.hpp"
# include "HTTP/HttpTypes.hpp"
# include "Utils/HashedTimingWheel.hpp"
# include "Utils/HashMap.hpp"
# include "Logger.hpp"

struct ServerLimits
{
	uint64_t	max_body_size;

	void	load(toml::Variant& table);
};

struct RouteConfig
{
	std::string	path;
	HandlerType	handler;
	bool		method_allowed[Method::length];

	std::string	root;
	std::string	alias;

	virtual ~RouteConfig() {};

	void	load(toml::Variant& table);
};

struct StaticConfig : public RouteConfig
{
	std::string	index;
	bool		autoindex;

	void	load(toml::Variant& table);
};

struct UploadConfig : public RouteConfig
{
	std::vector<MIME>	allowed_extensions;
	std::string			upload_store;
	uint64_t			max_file_size;
	bool				allow_overwrite;

	void	load(toml::Variant& table);
};

struct RedirectConfig : public RouteConfig
{
	std::string	redirect_location;
	HTTPCode	status;

	void	load(toml::Variant& table);
};

struct CGIConfig : public RouteConfig
{
	HashMap<std::string, std::string>	env;
	std::string							interpreter;
	std::string							bin;
	timestamp_ms						timeout;

	void	load(toml::Variant& table);
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

	void	load(toml::Variant& table);
	void	load_errors(toml::Table& errors_table);
};

struct EngineConfig
{
	size_t			max_events;
	timestamp_ms	max_timeout;
	timestamp_ms	closing_timeout;
	size_t			read_size;
	size_t			max_read_limit;

	void	load(toml::Variant& table);
};

struct LoggingConfig
{
	std::string	log_file;
	LogLevel	log_level;

	void	load(toml::Variant& table);
};

struct Config
{
	EngineConfig			engineConfig;
	LoggingConfig			loggingConfig;
	RadixTree<ServerConfig>	serversConfig;

	class Exception : public std::runtime_error
	{
	public:
		Exception(const std::string& msg) : std::runtime_error(std::string("Config : ") + msg) {}
	};

	void	load(toml::Document& document);
};

#endif // _CONFIGSTRUCTS_H