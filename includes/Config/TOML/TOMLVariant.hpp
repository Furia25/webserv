/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TOMLVariant.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/09 21:59:07 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/24 16:06:58 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _VARIANT_H
# define _VARIANT_H

# include <cstring>
# include <vector>

# include "Utils/Optional.hpp"
# include "Utils/HashMap.hpp"
# include <iostream>

namespace toml
{

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

	template <typename T>
	Variant&	operator= (const T);
	Variant&	operator= (const Variant& other);
	const Variant&	operator[](const std::string& key) const;

	# define X(name, T, ...) Variant& operator= (const T& value);
	_VARIANT_TYPES
	# undef X

	template <typename T> T&		as();
	template <typename T> const T&	as() const;
	template <typename T> T			get();
	template <typename T> T			get() const;
	template <typename T> T			take(const std::string& key);
	template <typename T> T			take_or(const std::string& key, const T& def);

	Variant				take_section(const std::string& key, bool optional);
	Variant				take_section_array(const std::string& key, bool optional);

	Type				getType() const;
	bool				isNone() const { return this->type == NONE; }
	bool				isImplicit() const { return this->context == IMPLICIT; };
	bool				isHeader() const { return this->context == HEADER; };
	bool				isExplicit() const { return this->context == EXPLICIT; };
	Variant&			setExplicit() { this->context = EXPLICIT; return *this; };
	Variant&			setHeader() { this->context = HEADER; return *this; };
	static Variant		null() { return Variant(); }

	const char			*toString();
	static const char	*toString(const Type type);

	class ParsedException : public std::runtime_error
	{
	public:
		ParsedException(const std::string& msg)
			: std::runtime_error(msg) {}
		ParsedException(const std::string& type, const std::string& msg)
			: std::runtime_error(type + ": " + msg) {}
		ParsedException(const Type type, const Type expected)
			: std::runtime_error(std::string("Invalid type: got ") + toString(type) + ", expected " + Variant::toString(expected)) {}
	};

protected:
private:
	enum Tag {IMPLICIT, EXPLICIT, HEADER};

	Type	type;
	Tag		context;
	union Data
	{
		# define X(name, T, ...) RawStorage<T>	name;
			_VARIANT_TYPES
		# undef X
	}	data;

	void		check_types_errors(Variant::Type expected) const;
	void		destruct();

	template <typename T> void	construct(const T& value);
};

inline void toml::Variant::check_types_errors(Variant::Type expected) const
{
	if (this->type != expected)
		throw Variant::ParsedException(this->type, expected);
}

# define X(name, T, ...) \
template <> \
inline void	toml::Variant::construct(const T& value) \
{ \
	this->check_types_errors(Variant::name); \
	this->data.name.construct(value); \
}
_VARIANT_TYPES
#undef X

template <>
inline void toml::Variant::construct(const Variant& other)
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
inline toml::Variant& toml::Variant::operator=(const T& value) \
{ \
	this->destruct(); \
	this->type = name; \
	this->construct(value); \
	return (*this); \
}
_VARIANT_TYPES
# undef X

template<>
inline toml::Variant& toml::Variant::operator=(const char *value)
{
	this->destruct();
	this->type = Variant::STRING;
	this->construct(std::string(value));
	return (*this);
}

template<>
inline toml::Variant& toml::Variant::operator=(const float value)
{
	this->destruct();
	this->type = Variant::FLOATING;
	this->construct(static_cast<double>(value));
	return (*this);
}

# define INTEGER_PROMOTION(int_type) \
template <> inline int_type		toml::Variant::get<int_type>() { this->check_types_errors(Variant::INTEGER); return (*this->data.INTEGER.ptr()); } \
template <> inline int_type	toml::Variant::get<int_type>() const { this->check_types_errors(Variant::INTEGER); return (*this->data.INTEGER.ptr()); } \
template<> \
inline toml::Variant& toml::Variant::operator=(const int_type value) \
{ \
	this->destruct(); \
	this->type = Variant::INTEGER; \
	this->construct(static_cast<long long>(value)); \
	return (*this); \
}

INTEGER_PROMOTION(char);
INTEGER_PROMOTION(int);
INTEGER_PROMOTION(short);
INTEGER_PROMOTION(long);
INTEGER_PROMOTION(uint64_t);

#define X(name, T) \
template <> inline T		toml::Variant::get<T>() { this->check_types_errors(Variant::name); return (*this->data.name.ptr()); } \
template <> inline T	toml::Variant::get<T>() const { this->check_types_errors(Variant::name); return (*this->data.name.ptr()); } \
template <> inline T&		toml::Variant::as<T>()	{ this->check_types_errors(Variant::name); return (*this->data.name.ptr()); } \
template <> inline const T&	toml::Variant::as<T>() const { this->check_types_errors(Variant::name); return (*this->data.name.ptr()); }
_VARIANT_TYPES
#undef X

template <> inline Variant&			toml::Variant::as<Variant>()	{ return *this; };
template <> inline const Variant&	toml::Variant::as<Variant>() const { return *this; };

typedef Variant::Array	Array;
typedef Variant::Table	Table;
typedef Variant			Value;

inline toml::Variant::Variant() : type(NONE), context(IMPLICIT) {}

inline toml::Variant::Variant(const Variant &other) : type(other.type), context(other.context)
{
	this->construct(other);
}

inline toml::Variant::~Variant()
{
	this->destruct();
}

inline toml::Variant& toml::Variant::operator=(const Variant &other)
{
	if (this == &other)
		return (*this);
	this->destruct();
	this->type = other.type;
	this->context = other.context;
	this->construct(other);
	return (*this);
}

inline const toml::Variant& toml::Variant::operator[](const std::string &key) const
{
	const toml::Table&	table = this->as<Table>();
	toml::Table::const_iterator it = table.find(key);
	if (it == table.end())
		throw toml::Variant::ParsedException("Unspecified Key");
	return it->second;
}

inline toml::Variant::Type toml::Variant::getType() const { return this->type; }

# define X(name, T, ...) \
inline toml::Variant::Variant(const T& value) : type(name), context(IMPLICIT) \
{ \
	this->construct(value); \
}
_VARIANT_TYPES

# undef X

inline toml::Variant::Variant(const char *value) : type(NONE) { *this = value; }
inline toml::Variant::Variant(const char value) : type(NONE) { *this = value; }
inline toml::Variant::Variant(const short value) : type(NONE) { *this = value; }
inline toml::Variant::Variant(const int value) : type(NONE) { *this = value; }
inline toml::Variant::Variant(const long value) : type(NONE) { *this = value; }
inline toml::Variant::Variant(const float value) : type(NONE) { *this = value; }

inline void	toml::Variant::destruct()
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

inline const char *toml::Variant::toString()
{
	return toml::Variant::toString(this->type);
}

inline const char *toml::Variant::toString(const Type type)
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

template <typename T>
inline T Variant::take(const std::string& key)
{
	Table&	table = this->as<Table>();
	Table::iterator	it = table.find(key);
	if (it == table.end())
		throw Variant::ParsedException("Missing key in table, \"" + key + '\"');
	T result = it->second.get<T>();
	table.erase(key);
	return result;
}

template <typename T>
inline T Variant::take_or(const std::string& key, const T& def)
{
	Table&	table = this->as<Table>();
	Table::iterator	it = table.find(key);
	if (it == table.end())
		return def;
	T result = it->second.get<T>();
	table.erase(key);
	return result;
}

inline Variant toml::Variant::take_section(const std::string& key, bool optional)
{
	Table&			table = this->as<Table>();
	Table::iterator	it = table.find(key);
	if (it == table.end())
	{
		if (!optional)
			throw Variant::ParsedException("Missing header section in table, \"" + key + '\"');
		return Variant(Table());
	}
	const Variant section = it->second;
	if (!section.isHeader() || section.type != TABLE)
		throw Variant::ParsedException("Expected header table from \"" + key + "\"");
	table.erase(it);
	return section;
}

inline Variant Variant::take_section_array(const std::string &key, bool optional)
{
	Table&			table = this->as<Table>();
	Table::iterator	it = table.find(key);
	if (it == table.end())
	{
		if (!optional)
			throw Variant::ParsedException("Missing header section in table, \"" + key + '\"');
		return Variant(Array());
	}
	const Variant section = it->second;
	if (!section.isHeader() || section.type != ARRAY)
		throw Variant::ParsedException("Expected header array of table from \"" + key + "\"");
	table.erase(it);
	return section;
}

};

#endif // _VARIANT_H
