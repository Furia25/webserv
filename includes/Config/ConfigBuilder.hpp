/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigBuilder.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 17:28:59 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/18 20:05:00 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _CONFIGLOADER_H
# define _CONFIGLOADER_H

# include <string>
# include <stdexcept>

# include "vector"
# include "toml.hpp"
# include "Config.hpp"
# include "ConfigBuilder.hpp"

class ConfigBuilder
{
public:
	ConfigBuilder();

	void	from_file(Config& to_build, const std::string& filepath);
	~ConfigBuilder();
protected:
private:
	std::vector<std::string>	exceptions;
};

#endif // _CONFIGLOADER_H