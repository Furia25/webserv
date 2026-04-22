/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/18 19:47:39 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/22 11:36:03 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _CONFIG_H
# define _CONFIG_H

# include "ConfigStructs.hpp"

struct Config
{
	EngineConfig			engineConfig;
	LoggingConfig			loggingConfig;
	RadixTree<ServerConfig>	serversConfig;

	Config() {}
	Config(const Config& src);
	~Config() {}

	Config& operator=(const Config& src);

	class Exception : public std::runtime_error
	{
	public:
		Exception(const std::string& msg) : std::runtime_error(std::string("Config : ") + msg) {}
	};
};

#endif // _CONFIG_H