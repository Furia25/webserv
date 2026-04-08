/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Variant.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 11:51:56 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/08 10:53:42 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _VARIANT_H
# define _VARIANT_H

# include <cstring>
# include <vector>

# include "macrosplosion.hpp"
# include "Optional.hpp"
# include "HashMap.hpp"

class Variant
{
public:

	typedef std::vector<Variant>			Array;
	typedef HashMap<std::string, Variant>	Table;

	# define _VARIANT_TYPES	\
		X(STRING, std::string, &) \
		X(BOOLEAN, bool, ) \
		X(INTEGER, long long, ) \
		X(FLOATING, double, ) \
		X(ARRAY, Variant::Array, &) \
		X(TABLE, Variant::Table, &)

	# define _VARIANT_NONE		X(NONE, , )

	# define X(name, T, ref, ...) name,
	enum Type { _VARIANT_TYPES _VARIANT_NONE};
	# undef X

	Variant();
	Variant(const Variant& other);
	~Variant();

	# define X(name, T, ref, ...) 	explicit Variant(const T ref value);
	_VARIANT_TYPES
	# undef X

	Variant&	operator= (const Variant& other);

	# define X(name, T, ref, ...) 	Variant&	operator= (const T ref value);
	_VARIANT_TYPES
	# undef X

	Type							getType() const;

	template <typename T> T			as();
	template <typename T> const T	as() const;

	static const char	*toString(const Type type);
protected:
private:
	Type	type;
	union Data
	{
		# define X(name, T, ref, ...) RawStorage<T>	name;
			_VARIANT_TYPES
		# undef X
	}	data;

	void	check_types_errors(Variant::Type expected);
	void	destruct();
	template <typename T> void	construct(const T& value);
};

class ParsedVariantException : public std::runtime_error
{
public:
	ParsedVariantException(const std::string& msg)
		: std::runtime_error(msg) {}
	ParsedVariantException(const std::string& type, const std::string& msg)
		: std::runtime_error(type + ": " + msg) {}
	ParsedVariantException(const Variant::Type type, const Variant::Type expected)
		: std::runtime_error(std::string("Invalid type access : got ") + Variant::toString(type) + ", expected " + Variant::toString(expected)) {}
};

inline void Variant::check_types_errors(Variant::Type expected)
{
	if (this->type != expected)
		throw ParsedVariantException(this->type, expected);
}

# define X(name, T, ref, ...) \
template <> \
inline void	Variant::construct(const T& value) \
{ \
	this->check_types_errors(Type::name); \
	this->data.name.construct(value); \
}

_VARIANT_TYPES

#undef X

template <>
inline void Variant::construct(const Variant& other)
{
	#define X(name, T, ref, ...) case name: this->data.name.construct(*other.data.name.ptr()); break;
	switch (this->type)
	{
		_VARIANT_TYPES
	default:
		break;
	};
	# undef X
}

# define X(name, T, ref, ...) \
inline Variant& Variant::operator=(const T ref value) \
{ \
	this->destruct(); \
	this->type = name; \
	this->construct(value); \
	return (*this); \
}

_VARIANT_TYPES

# undef X

# define X(name, T, ref, ...) \
template <> inline T ref	Variant::as() { this->check_types_errors(Type::name); return (*this->data.name.ptr()); }\
template <> inline const T ref	Variant::as() { this->check_types_errors(Type::name); return (*this->data.name.ptr());}

_VARIANT_TYPES

# undef X

#endif // _VARIANT_H

