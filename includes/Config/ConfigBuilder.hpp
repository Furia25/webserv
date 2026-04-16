/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigBuilder.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 17:28:59 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/16 19:34:24 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _CONFIGLOADER_H
# define _CONFIGLOADER_H

# include <string>
# include <stdexcept>

# include "ConfigStructs.hpp"
# include "toml.hpp"

struct Config
{
	EngineConfig	engineConfig;
	LoggingConfig	loggingConfig;
	std::vector<ServerConfig>	servers;

	void	from_file(const std::string& toml_file);

	class Exception : public std::runtime_error
	{
	public:
		Exception(const std::string& msg) : std::runtime_error(std::string("Config : ") + msg) {}
	};
};

#endif // _CONFIGLOADER_H