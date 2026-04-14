/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TOMLToken.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/08 13:47:04 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/14 13:24:08 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "TOMLToken.hpp"

void toml::Token::swap(Token& other)
{
	std::swap(this->type, other.type);
	std::swap(this->line, other.line);
	std::swap(this->col, other.col);
	this->literal.swap(other.literal);
}

toml::Token &toml::Token::operator=(const Token &other)
{
	this->type = other.type;
	this->line = other.line;
	this->literal = other.literal;
	this->col = other.col;
	return (*this);
}

const char *toml::Token::toString() const
{
	return Token::toString(this->type);
}

const char *toml::Token::toString(Token::Type type)
{
	switch (type)
	{
		#define X(x, ...) case Token::x: return "[" #x "]"; break;
		_TOML_TOKEN_TYPES_
		#undef X
	}
	return "Unknown";
}

toml::Token::Type toml::Token::getType() const { return type; }

const std::string &toml::Token::getLiteral() const { return literal; }

bool toml::is_whitespace(char c)
{
	return c == '\t' || c == ' ';
}

bool toml::is_control(char c)
{
	return (c <= 0x08) || (c >= 0x0A && c <= 0x1F) || (c == 0x7F);
}
