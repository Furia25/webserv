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

# include "Config/toml.hpp"
# include "Config/TOML/TOMLParser.hpp"

toml::Document::Document() : root(toml::Table()) {}

toml::Document::Document(const std::string& path)
{
	this->from_file(path);
}

toml::Document::Document(std::istream& stream, const std::string& name)
{
	this->from_stream(stream, false, name);
}

toml::Document::Document(const Document& other) { *this = other; }

toml::Document::~Document() {}

toml::Document& toml::Document::operator=(const Document& other)
{
	if (this == &other)
		return (*this);
	this->root = other.root;
	return (*this);
}

void toml::Document::from_stream(std::istream &stream, bool append, const std::string& name)
{
	if (!append)
		this->root = toml::Table();
	toml::TOMLErrorManager	error_manager(name);
	toml::TOMLParser		parser(stream, *this, error_manager);
}

void toml::Document::from_file(const std::string &path, bool append)
{
	std::ifstream	file;
	file.open(path.c_str());
	if (!file)
		throw std::runtime_error("TOML: could not open '" + path + "': " + std::strerror(errno));
	from_stream(file, append, path);
}

const toml::Value& toml::Document::operator[](const std::string &key) const
{
	return this->root[key];
}

bool toml::Document::contain(const std::string &key) const
{
	return this->root.as<toml::Table>().contain(key);
}

toml::Value& toml::Document::getRoot() { return this->root; }
