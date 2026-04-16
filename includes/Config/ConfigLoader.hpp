/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigLoader.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 17:28:59 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/16 18:55:58 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _CONFIGLOADER_H
# define _CONFIGLOADER_H

# include <string>

# include "Config.hpp"

# include "toml.hpp"

namespace ConfigBuilder
{
	void	from_file(Config& config, const std::string& toml_file);
};

#endif // _CONFIGLOADER_H