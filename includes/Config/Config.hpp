/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/22 23:26:37 by vdurand           #+#    #+#             */
/*   Updated: 2026/05/12 16:09:36 by vdurand          ###   ########.fr       */
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

# include "EnumClass.hpp"
# include "Utils/RadixTree.hpp"
# include "HTTP/HttpTypes.hpp"
# include "Utils/HashedTimingWheel.hpp"
# include "Utils/HashMap.hpp"
# include "Logger.hpp"

namespace Config
{

struct ServerConfig;

struct CookieConfig
{
	# define _SAMESITE_ (LAX, STRICT, NONE)
	ENUM_CLASS(SameSite, _SAMESITE_, ENUM_BASIC, ENUM_LITERALS(_SAMESITE_, ENUM_BASIC, ENUM_BASIC); public: SameSite() : _t(LAX) {});
	# undef _SAMESITE_

	std::string	name;

	uint64_t	max_age;
	bool		http_only;
	SameSite	same_site;

	bool		generate;
	size_t		generation_length;
	std::string	default_value;

	bool		required;

	void		load(toml::Variant& table, Config::Loader& loader);
};

struct RouteConfig
{
	HandlerType	handler;
	std::string	path;
	bool		method_allowed[Method::length];
	uint64_t	max_body_size;

	std::string	root;
	std::string	alias;

	HashMap<std::string, CookieConfig>	cookies;

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

	~StaticConfig() {};
};

struct UploadConfig : public RouteConfig
{
	std::vector<MIME>	allowed_extensions;
	std::string			upload_store;
	bool				allow_overwrite;

	UploadConfig(const ServerConfig *server_config) : RouteConfig(server_config) {};

	void	loadChild(toml::Variant& table, Config::Loader& loader);
	void	loadAllowedExtensions(toml::Variant& table, Config::Loader& loader);

	~UploadConfig() {};
};

struct RedirectConfig : public RouteConfig
{
	std::string	redirect_location;
	HTTPCode	status;

	RedirectConfig(const ServerConfig *server_config) : RouteConfig(server_config) {};

	void	loadChild(toml::Variant& table, Config::Loader& loader);

	~RedirectConfig() {};
};

struct CGIConfig : public RouteConfig
{
	HashMap<std::string, std::string>	env;
	toml::Table							interpreters;
	std::string							default_bin;
	timestamp_ms						timeout;

	CGIConfig(const ServerConfig *server_config) : RouteConfig(server_config) {};

	void	loadChild(toml::Variant& table, Config::Loader& loader);
};

struct StatusConfig : public RouteConfig
{
	bool	start_time;
	bool	server_info;
	bool	connection_info;
	bool	request_info;
	bool	timestamp;

	StatusConfig(const ServerConfig *server_config) : RouteConfig(server_config) {};

	void	loadChild(toml::Variant& table, Config::Loader& loader);

	~StatusConfig() {};
};

struct ServerConfig
{
	std::string		name;
	std::string		root;
	uint64_t		max_body_size;

	std::vector<std::pair<std::string, port_t> >	bindings;
	RadixTree<RouteConfig *>						routes;
	HashMap<HTTPCode, std::string>					error_fallbacks;

	HashMap<std::string, CookieConfig>	cookies;

	~ServerConfig();

	void	load(toml::Variant& table, Config::Loader& loader);
	void	loadBindings(toml::Array& bindings_table, Config::Loader& loader);
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

	~EngineConfig() {};
};

struct LoggingConfig
{
	std::string	log_file;
	LogLevel	log_level;
	uint64_t	tick_interval;

	void	load(toml::Variant& table, Config::Loader& loader);

	~LoggingConfig() {};
};

struct AppConfig
{
	AppConfig(const std::string& filename);
	EngineConfig								engineConfig;
	LoggingConfig								loggingConfig;
	HashMap<port_t, RadixTree<ServerConfig *> >	serversMap;
	std::vector<ServerConfig *>					servers;

	~AppConfig();
};

class Exception : public std::runtime_error
{
public:
	Exception(const std::string& msg) : std::runtime_error(std::string("Config: ") + msg) {}
};

} // namespace Config

#endif // _CONFIGSTRUCTS_H