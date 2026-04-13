/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Variant.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/08 08:53:50 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/13 21:55:16 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "TOMLVariant.hpp"

toml::Variant::Variant() : type(NONE), is_explicit(false) {}

toml::Variant::Variant(const Variant &other) : type(other.type), is_explicit(false)
{
	this->construct(other);
}

toml::Variant::~Variant()
{
	this->destruct();
}

toml::Variant& toml::Variant::operator=(const Variant &other)
{
	if (this == &other)
		return (*this);
	this->destruct();
	this->type = other.type;
	this->construct(other);
	return (*this);
}

toml::Variant::Type toml::Variant::getType() const { return this->type; }

# define X(name, T, ...) \
toml::Variant::Variant(const T& value) : type(name), is_explicit(false) \
{ \
	this->construct(value); \
}
_VARIANT_TYPES

# undef X

toml::Variant::Variant(const char *value) : type(NONE) { *this = value; }
toml::Variant::Variant(const char value) : type(NONE) { *this = value; }
toml::Variant::Variant(const short value) : type(NONE) { *this = value; }
toml::Variant::Variant(const int value) : type(NONE) { *this = value; }
toml::Variant::Variant(const long value) : type(NONE) { *this = value; }
toml::Variant::Variant(const float value) : type(NONE) { *this = value; }

void	toml::Variant::destruct()
{
	#define X(name, T, ...) case name: this->data.name.destroy(); break;
	switch (this->type)
	{
		_VARIANT_TYPES
	default:
		break;
	};
	# undef X
}

const char *toml::Variant::toString(const Type type)
{
	#define X(name, T, ...) case name: return #name; break;
	switch (type)
	{
		_VARIANT_TYPES
		_VARIANT_NONE
	default:
		break;
	};
	return "Unknown";
	# undef X
}
