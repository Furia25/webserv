/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/22 11:35:08 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/22 11:36:30 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Config/Config.hpp"

Config::Config(const Config& src) : engineConfig(src.engineConfig), 
loggingConfig(src.loggingConfig), serversConfig(src.serversConfig) 
{ 
}


Config& Config::operator=(const Config& rhs)
{
	if (this != &rhs)
	{
		engineConfig = rhs.engineConfig;
		loggingConfig = rhs.loggingConfig;
		serversConfig = rhs.serversConfig;
	}
	return *this;
}
