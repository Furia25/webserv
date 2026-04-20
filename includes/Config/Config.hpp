/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/18 19:47:39 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/20 16:21:22 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _CONFIG_H
# define _CONFIG_H

# include "ConfigStructs.hpp"

struct Config
{
	Config() {};
	EngineConfig			engineConfig;
	LoggingConfig			loggingConfig;
	RadixTree<ServerConfig>	serversConfig;

	class Exception : public std::runtime_error
	{
	public:
		Exception(const std::string& msg) : std::runtime_error(std::string("Config : ") + msg) {}
	};
};

#endif // _CONFIG_H