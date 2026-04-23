/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IJob.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 18:34:43 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/23 13:55:57 by antbonin         ###   ########.fr       */
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
