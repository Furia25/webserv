/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TOMLToken.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/08 13:47:04 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/09 17:49:55 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "TOMLToken.hpp"

TOML::Token::Type TOML::Token::getType() const
{
	return type;
}

const Variant& TOML::Token::getValue() const
{
	return value;
}

bool TOML::is_whitespace(char c)
{
	return c == '\t' || c == ' ';
}

bool TOML::is_control(char c)
{
	return (c <= 0x08) || (c >= 0x0A && c <= 0x1F) || (c == 0x7F);
}

