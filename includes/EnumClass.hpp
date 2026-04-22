/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EnumClass.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 16:05:08 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/23 01:11:17 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _ENUMCLASS_H
# define _ENUMCLASS_H

# include "macrosplosion.hpp"
# include <exception>
# include <cstring>

#define ENUM_BASIC(val, ...) val __VA_ARGS__
#define ENUM_BASIC_STRING(val, ...) val __VA_ARGS__

#define ENUM_CLASS(name, enum_tuple, enum_macro, ...)\
struct name {\
public:\
	enum E {\
		M_TUPLE_FOREACH(enum_tuple, enum_macro, M_COMMA())\
	};\
	name(E e) : _t(e) {};\
	operator E() const { return _t; }\
	E operator()() const {return _t;}\
	static const size_t	length = M_TUPLE_SIZE(enum_tuple);\
	bool operator==(const name& other) const { return _t == other._t; }\
	bool operator!=(const name& other) const { return _t != other._t; }\
	bool operator<(const name& other) const { return _t < other._t; }\
	bool operator<=(const name& other) const { return _t <= other._t; }\
	bool operator>(const name& other) const { return _t > other._t; }\
	bool operator>=(const name& other) const { return _t >= other._t; }\
	bool operator==(E e) const { return _t == e; }\
\
__VA_ARGS__ \
\
private:\
	E _t;\
}

#define _ENUM_CLASS_STRING_CASE(el, enum_macro, string_macro, ...) case enum_macro(el): return (M_STR(string_macro(el, __VA_ARGS__))); break;
#define _ENUM_CLASS_STRING_CMP(el, enum_macro, string_macro, ...)\
else if (std::strcmp(M_STR(string_macro(el, __VA_ARGS__)), c_str) == 0)\
	return (enum_macro(el));

#define _ENUM_CLASS_STRING_ERROR(c_str)	\
	throw std::domain_error(std::string("Illegal conversion from string to enum, str: ") + "'" + c_str + "'" + " (function: " + __PRETTY_FUNCTION__ + ")")

#define ENUM_LITERALS(enum_tuple, enum_macro, string_macro, ...)\
public:\
	static const char *toString(size_t e)\
	{\
		return (toString(static_cast<E>(e)));\
	}\
	static const char *toString(E e)\
	{\
		switch (e)\
		{\
			M_TUPLE_FOREACH(enum_tuple, _ENUM_CLASS_STRING_CASE, enum_macro, string_macro, __VA_ARGS__)\
			default:\
				return (unknown());\
		}\
	}\
	inline std::string methodName(const std::string& prettyFunction)\
	{\
		size_t colons = prettyFunction.find("::");\
		size_t begin = prettyFunction.substr(0,colons).rfind(" ") + 1;\
		size_t end = prettyFunction.rfind("(") - begin;\
		return prettyFunction.substr(begin,end) + "()";\
	}\
	static E	from(const char *c_str)\
	{\
		if (c_str == NULL)\
			_ENUM_CLASS_STRING_ERROR(c_str);\
		M_TUPLE_FOREACH(enum_tuple, _ENUM_CLASS_STRING_CMP,  enum_macro, string_macro, __VA_ARGS__)\
		else\
			_ENUM_CLASS_STRING_ERROR(c_str);\
		return (static_cast<E>(0));\
	}\
	static E	from(const std::string& str)\
	{\
		return (from(str.c_str()));\
	}\
private:\
	static const char *unknown() {\
		return ("Unknown");\
	} \

#endif // _ENUMCLASS_H