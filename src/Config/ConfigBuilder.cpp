/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigBuilder.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 18:34:43 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/18 20:08:13 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigBuilder.hpp"

ConfigBuilder::ConfigBuilder()
{
}

void ConfigBuilder::from_file(Config& to_build, const std::string& filepath)
{
	toml::Document	document;

	document.from_file(filepath);
}

ConfigBuilder::~ConfigBuilder()
{
}
