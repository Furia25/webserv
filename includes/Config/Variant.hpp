/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Variant.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/09 21:59:07 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/09 22:01:37 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _VARIANT_H
# define _VARIANT_H

# include <cstring>
# include <vector>

# include "Optional.hpp"
# include "HashMap.hpp"

class Variant
{
public:
	typedef std::vector<Variant>			Array;
	typedef HashMap<std::string, Variant>	Table;
	# define _VARIANT_TYPES	\
		X(STRING, std::string) \
		X(BOOLEAN, bool) \
		X(INTEGER, long long) \
		X(FLOATING, double) \
		X(ARRAY, Variant::Array) \
		X(TABLE, Variant::Table)

	# define _VARIANT_NONE		X(NONE, , )

	# define X(name, T, ...) name,
	enum Type { _VARIANT_TYPES _VARIANT_NONE};
	# undef X

	Variant();
	Variant(const Variant& other);
	Variant(const char *value);
	Variant(const char value);
	Variant(const short value);
	Variant(const int value);
	Variant(const long value);
	Variant(const float value);
	~Variant();

	# define X(name, T, ...) 	explicit Variant(const T& value);
	_VARIANT_TYPES
	# undef X

	Variant&	operator= (const Variant& other);
	template <typename T>
	Variant&	operator= (const T);
	# define X(name, T, ...) 	Variant&	operator= (const T& value);
	_VARIANT_TYPES
	# undef X

	Type							getType() const;

	template <typename T> T&		as();
	template <typename T> const T&	as() const;

	bool				isNone() const { return this->type == NONE; }
	static Variant		null() { return Variant(); }

	std::string			*toString();
	static const char	*toString(const Type type);
protected:
private:
	Type	type;
	union Data
	{
		# define X(name, T, ...) RawStorage<T>	name;
			_VARIANT_TYPES
		# undef X
	}	data;

	void	check_types_errors(Variant::Type expected) const;
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

inline void Variant::check_types_errors(Variant::Type expected) const
{
	if (this->type != expected)
		throw ParsedVariantException(this->type, expected);
}

# define X(name, T, ...) \
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
inline Variant& Variant::operator=(const T& value) \
{ \
	this->destruct(); \
	this->type = name; \
	this->construct(value); \
	return (*this); \
}
_VARIANT_TYPES
# undef X

template<>
inline Variant& Variant::operator=(const char *value)
{
	this->destruct();
	this->type = Variant::Type::STRING;
	this->construct(std::string(value));
	return (*this);
}

template<>
inline Variant& Variant::operator=(const float value)
{
	this->destruct();
	this->type = Variant::Type::FLOATING;
	this->construct(static_cast<double>(value));
	return (*this);
}

# define INTEGER_PROMOTION(int_type) \
template<> \
inline Variant& Variant::operator=(const int_type value) \
{ \
	this->destruct(); \
	this->type = Variant::Type::INTEGER; \
	this->construct(static_cast<long long>(value)); \
	return (*this); \
}

INTEGER_PROMOTION(char);
INTEGER_PROMOTION(int);
INTEGER_PROMOTION(short);
INTEGER_PROMOTION(long);

#define X(name, T) \
template <> inline T&		Variant::as<T>()	{ this->check_types_errors(Variant::Type::name); return (*this->data.name.ptr()); } \
template <> inline const T&	Variant::as<T>() const { this->check_types_errors(Variant::Type::name); return (*this->data.name.ptr()); }
_VARIANT_TYPES
#undef X

#endif // _VARIANT_H
