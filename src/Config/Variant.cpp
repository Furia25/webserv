/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Variant.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/08 08:53:50 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/09 22:02:03 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Variant.hpp"

Variant::Variant() : type(NONE) {}

Variant::Variant(const Variant &other) : type(other.type)
{
	this->construct(other);
}

Variant::~Variant()
{
	this->destruct();
}

Variant& Variant::operator=(const Variant &other)
{
	if (this == &other)
		return (*this);
	this->destruct();
	this->type = other.type;
	this->construct(other);
	return (*this);
}

Variant::Type Variant::getType() const
{
	return this->type;
}

# define X(name, T, ...) \
Variant::Variant(const T& value) : type(name) \
{ \
	this->construct(value); \
}
_VARIANT_TYPES
# undef X

Variant::Variant(const char *value) : type(NONE) { *this = value; }
Variant::Variant(const char value) : type(NONE) { *this = value; }
Variant::Variant(const short value) : type(NONE) { *this = value; }
Variant::Variant(const int value) : type(NONE) { *this = value; }
Variant::Variant(const long value) : type(NONE) { *this = value; }
Variant::Variant(const float value) : type(NONE) { *this = value; }

void	Variant::destruct()
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

const char *Variant::toString(const Type type)
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
