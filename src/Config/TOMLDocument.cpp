/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TOMLDocument.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/10 16:31:26 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/14 00:32:35 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "TOMLDocument.hpp"
# include  "TOMLParser.hpp"

void toml::Document::from_stream(std::istream &stream, bool append)
{
	if (!append)
		this->root = toml::Table();
	toml::TOMLParser	parser(stream, *this);
}

void toml::Document::from_file(const std::string &path, bool append)
{
	std::ifstream	file(path);
	if (!file)
		throw std::runtime_error("TOML: could not open '" + path + "': " + std::strerror(errno));
	from_stream(file, append);
}

toml::Value& toml::Document::getRoot() { return this->root; }
