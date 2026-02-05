/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EnumClass.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 16:05:08 by vdurand           #+#    #+#             */
/*   Updated: 2026/02/05 18:34:26 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _ENUMCLASS_H
# define _ENUMCLASS_H

# include <exception>
# include <string.h>
# include <map>

#define ENUM_CLASS_ENUM(val, ...) val,

#define ENUM_CLASS(name, enum_tuple, enum_macro, ...)\
struct name {\
public:\
	enum E {\
		M_TUPLE_FOREACH(enum_tuple, enum_macro, ~)\
	};\
	name(E e) : _t(e) {};\
	E operator()() const {return _t;}\
	static size_t	length() {return (M_TUPLE_SIZE(enum_tuple));};\
\
__VA_ARGS__ \
\
private:\
	E _t;\
}

#define _ENUM_CLASS_STRING_CASE(tuple, string_macro, ...) case M_TUPLE_ELEMENT(tuple, 0): return (M_STR(string_macro(tuple, __VA_ARGS__))); break;
#define _ENUM_CLASS_STRING_CMP(tuple, string_macro, ...)\
else if (strcmp(M_STR(string_macro(tuple, __VA_ARGS__)), c_str) == 0)\
	return (M_TUPLE_ELEMENT(tuple, 0));

#define _ENUM_CLASS_STRING_ERROR(c_str)	\
	throw std::domain_error(std::string("Illegal conversion from string to enum, str: ") + "'" + c_str + "'" + " (function: " + __PRETTY_FUNCTION__ + ")")

#define ENUM_LITERALS(enum_tuple, string_macro, ...)\
public:\
	static const char *toString(size_t e)\
	{\
		switch (static_cast<E>(e))\
		{\
			M_TUPLE_FOREACH(enum_tuple, _ENUM_CLASS_STRING_CASE, string_macro, __VA_ARGS__)\
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
		M_TUPLE_FOREACH(enum_tuple, _ENUM_CLASS_STRING_CMP, string_macro, __VA_ARGS__)\
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