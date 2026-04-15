/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigStructs.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 17:53:32 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/15 21:57:29 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _CONFIGSTRUCTS_H
# define _CONFIGSTRUCTS_H

# include <string>

# include "Optional.hpp"
# include "HTTP/HttpTypes.hpp"
# include "HashedTimingWheel.hpp"

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
};

struct RedirectConfig : public RouteConfig
{
	
};

struct CGIConfig : public RouteConfig
{
	std::string		cgi_path;
	timestamp_ms	timeout;
};

struct ServerConfig
{
	std::string	name;
	std::string	host;
	std::string	service;
	std::string	root;
};

#endif // _CONFIGSTRUCTS_H