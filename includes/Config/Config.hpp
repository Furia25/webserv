/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/18 19:47:39 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/18 20:03:47 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _CONFIG_H
# define _CONFIG_H

# include "ConfigStructs.hpp"

struct Config
{
	EngineConfig	engineConfig;
	LoggingConfig	loggingConfig;
	std::vector<ServerConfig>	servers;

	class Exception : public std::runtime_error
	{
	public:
		Exception(const std::string& msg) : std::runtime_error(std::string("Config : ") + msg) {}
	};
};

#endif // _CONFIG_H