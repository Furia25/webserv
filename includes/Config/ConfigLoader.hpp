/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigLoader.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 17:28:59 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/15 18:10:34 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _CONFIGLOADER_H
# define _CONFIGLOADER_H

# include <string>

# include "toml.hpp"

class ConfigLoader
{
public:
protected:
private:
	toml::Document	config;
};

#endif // _CONFIGLOADER_H