/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   toml.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/10 16:31:26 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/16 18:40:47 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "toml.hpp"
# include "TOMLParser.hpp"

toml::Document::Document() {}

toml::Document::Document(const Document& other) {*this = other;}

toml::Document::~Document() {}

toml::Document& toml::Document::operator=(const Document& other)
{
	if (this == &other)
		return (*this);
	this->root = other.root;
	return (*this);
}

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
