/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RouteVariant.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/09 21:59:07 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/23 00:39:03 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _VARIANT_H
# define _VARIANT_H

# include <cstring>
# include <vector>

# include "Config.hpp"
# include "Utils/Optional.hpp"
# include "Utils/HashMap.hpp"
# include <iostream>

namespace Config
{

class RouteVariant
{
public:
	# define _VARIANT_TYPES	\
		X(STATIC, StaticConfig) \
		X(UPLOAD, UploadConfig) \
		X(CGI, CGIConfig) \
		X(REDIRECT, RedirectConfig) \
		X(STATUS, StatusConfig)

	# define _VARIANT_NONE		X(NONE, , )

	# define X(name, T, ...) name,
	enum Type { _VARIANT_TYPES _VARIANT_NONE};
	# undef X

	RouteVariant();
	RouteVariant(const RouteVariant& other);
	RouteVariant(const char *value);
	RouteVariant(const char value);
	RouteVariant(const short value);
	RouteVariant(const int value);
	RouteVariant(const long value);
	RouteVariant(const float value);
	~RouteVariant();

	# define X(name, T, ...) 	explicit RouteVariant(const T& value);
	_VARIANT_TYPES
	# undef X

	template <typename T>
	RouteVariant&	operator= (const T);
	RouteVariant&	operator= (const RouteVariant& other);

	# define X(name, T, ...) RouteVariant& operator= (const T& value);
	_VARIANT_TYPES
	# undef X

	template <typename T> T&		as();
	template <typename T> const T&	as() const;

	Type					getType() const;
	bool					isNone() const { return this->type == NONE; }
	static RouteVariant		null() { return RouteVariant(); }
	const char				*toString();
	static const char		*toString(const Type type);

	class ParsedException : public std::runtime_error
	{
	public:
		ParsedException(const std::string& msg)
			: std::runtime_error(msg) {}
		ParsedException(const std::string& type, const std::string& msg)
			: std::runtime_error(type + ": " + msg) {}
		ParsedException(const Type type, const Type expected)
			: std::runtime_error(std::string("Invalid type: got ") + toString(type) + ", expected " + RouteVariant::toString(expected)) {}
	};

protected:
private:
	enum Tag {IMPLICIT, EXPLICIT, HEADER};

	Type	type;
	union Data
	{
		# define X(name, T, ...) RawStorage<T>	name;
			_VARIANT_TYPES
		# undef X
	}	data;

	void		check_types_errors(RouteVariant::Type expected) const;
	void		destruct();

	template <typename T> void	construct(const T& value);
};

inline void Config::RouteVariant::check_types_errors(RouteVariant::Type expected) const
{
	if (this->type != expected)
		throw RouteVariant::ParsedException(this->type, expected);
}

# define X(name, T, ...) \
template <> \
inline void	Config::RouteVariant::construct(const T& value) \
{ \
	this->check_types_errors(Type::name); \
	this->data.name.construct(value); \
}
_VARIANT_TYPES
#undef X

template <>
inline void Config::RouteVariant::construct(const RouteVariant& other)
{
	#define X(name, T, ...) case name: this->data.name.construct(*other.data.name.ptr()); break;
	switch (this->type)
	{
		_VARIANT_TYPES
	default:
		break;
	};
	# undef X
}

# define X(name, T, ...) \
inline Config::RouteVariant& Config::RouteVariant::operator=(const T& value) \
{ \
	this->destruct(); \
	this->type = name; \
	this->construct(value); \
	return (*this); \
}
_VARIANT_TYPES
# undef X

#define X(name, T) \
template <> inline T&		Config::RouteVariant::as<T>()	{ this->check_types_errors(RouteVariant::Type::name); return (*this->data.name.ptr()); } \
template <> inline const T&	Config::RouteVariant::as<T>() const { this->check_types_errors(RouteVariant::Type::name); return (*this->data.name.ptr()); }
_VARIANT_TYPES
#undef X

template <> inline RouteVariant&			Config::RouteVariant::as<RouteVariant>()	{ return *this; };
template <> inline const RouteVariant&	Config::RouteVariant::as<RouteVariant>() const { return *this; };

inline Config::RouteVariant::RouteVariant() : type(NONE){}

inline Config::RouteVariant::RouteVariant(const RouteVariant &other) : type(other.type)
{
	this->construct(other);
}

inline Config::RouteVariant::~RouteVariant()
{
	this->destruct();
}

inline Config::RouteVariant& Config::RouteVariant::operator=(const RouteVariant &other)
{
	if (this == &other)
		return (*this);
	this->destruct();
	this->type = other.type;
	this->construct(other);
	return (*this);
}

Config::RouteVariant::Type Config::RouteVariant::getType() const { return this->type; }

# define X(name, T, ...) \
Config::RouteVariant::RouteVariant(const T& value) : type(name) \
{ \
	this->construct(value); \
}
_VARIANT_TYPES

# undef X

Config::RouteVariant::RouteVariant(const char *value) : type(NONE) { *this = value; }
Config::RouteVariant::RouteVariant(const char value) : type(NONE) { *this = value; }
Config::RouteVariant::RouteVariant(const short value) : type(NONE) { *this = value; }
Config::RouteVariant::RouteVariant(const int value) : type(NONE) { *this = value; }
Config::RouteVariant::RouteVariant(const long value) : type(NONE) { *this = value; }
Config::RouteVariant::RouteVariant(const float value) : type(NONE) { *this = value; }

void	Config::RouteVariant::destruct()
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

const char *Config::RouteVariant::toString()
{
	return Config::RouteVariant::toString(this->type);
}

const char *Config::RouteVariant::toString(const Type type)
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

};

#endif // _VARIANT_H
