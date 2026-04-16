/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigLoader.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 18:34:43 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/16 18:39:04 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigLoader.hpp"

void ConfigBuilder::from_file(Config& config, const std::string &toml_file)
{
	toml::Document	document;

	document.from_file(toml_file, false);
}
