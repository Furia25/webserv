/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigStructs.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 17:53:32 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/16 19:48:12 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _CONFIGSTRUCTS_H
# define _CONFIGSTRUCTS_H

# include <string>

# include "ConfigDefault.hpp"

# include "Optional.hpp"
# include "RadixTree.hpp"
# include "HTTP/HttpTypes.hpp"
# include "HashedTimingWheel.hpp"
# include "HashMap.hpp"
# include "Logger.hpp"

struct ServerLimits
{
	uint64_t	max_connections;
	uint64_t	max_body_size;
	uint8_t		request_timeout;
};

struct RouteConfig
{
	std::string	path;
	HandlerType	handler;
	bool		method_allowed[Method::length];

	std::string	root;
	std::string	alias;

	virtual ~RouteConfig() {}
};

struct StaticConfig : public RouteConfig
{
	std::string	index;
	bool		autoindex;	
};

struct UploadConfig : public RouteConfig
{
	std::vector<MIME>	allowed_extensions;
	std::string			upload_store;
	uint64_t			max_file_size;
	bool				allow_overwrite;
};

struct RedirectConfig : public RouteConfig
{
	std::string	redirect_location;
	HTTPCode	status;
};

struct CGIConfig : public RouteConfig
{
	HashMap<std::string, std::string>	env;
	std::string							interpreter;
	std::string							bin;
	timestamp_ms						timeout;
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
};

struct EngineConfig
{
	size_t			max_events;
	timestamp_ms	max_timeout;
	timestamp_ms	closing_timeout;
	size_t			read_size;
	size_t			max_read_limit;
};

struct LoggingConfig
{
	std::string	log_file;
	LogLevel	log_level;
};

#endif // _CONFIGSTRUCTS_H