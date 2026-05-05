/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EnumClass.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 16:05:08 by vdurand           #+#    #+#             */
/*   Updated: 2026/05/05 18:02:56 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _ENUMCLASS_H
# define _ENUMCLASS_H

# include "Utils/RadixTree.hpp"
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
	\
	bool operator==(const E& e) const { return _t == e; }\
	bool operator!=(const E& e) const { return _t != e; }\
	bool operator<(const E& e) const { return _t < e; }\
	bool operator<=(const E& e) const { return _t <= e; }\
	bool operator>(const E& e) const { return _t > e; }\
	bool operator>=(const E& e) const { return _t >= e; }\
\
__VA_ARGS__ \
\
private:\
	E _t;\
}

#define _ENUM_CLASS_TREE_INSERT(el, enum_macro, string_macro, ...) t.insert(M_STR(string_macro(el, __VA_ARGS__)), enum_macro(el));
#define _ENUM_CLASS_STRING_CASE(el, enum_macro, string_macro, ...) case enum_macro(el): return (M_STR(string_macro(el, __VA_ARGS__))); break;

#define ENUM_CLASS_STRING_ERROR(c_str)	\
	throw std::domain_error(std::string("Illegal conversion from string to enum, str: ") + "'" + c_str + "'" + " (function: " + __PRETTY_FUNCTION__ + ")")

#define ENUM_LITERALS(enum_tuple, enum_macro, string_macro, ...) \
private:\
	static const RadixTree<E>& getTree() \
	{ \
		static RadixTree<E> t; \
		static bool initialized = false; \
		if (!initialized) \
		{ \
			M_TUPLE_FOREACH(enum_tuple, _ENUM_CLASS_TREE_INSERT, enum_macro, string_macro, __VA_ARGS__) \
			initialized = true; \
		} \
		return t; \
	} \
public:\
	static const char *toString(size_t e)\
	{\
		return (toString(static_cast<E>(e)));\
	}\
	static const char *toString(E e)\
	{\
		switch (e)\
		{\
			M_TUPLE_FOREACH(enum_tuple, _ENUM_CLASS_STRING_CASE, enum_macro, string_macro, __VA_ARGS__) \
			default: \
				return (unknown()); \
		}\
	}\
private:\
	inline std::string methodName(const std::string& prettyFunction)\
	{\
		size_t colons = prettyFunction.find("::");\
		size_t begin = prettyFunction.substr(0,colons).rfind(" ") + 1;\
		size_t end = prettyFunction.rfind("(") - begin;\
		return prettyFunction.substr(begin,end) + "()";\
	}\
public:\
	static E	from(const char *c_str)\
	{\
		if (c_str == NULL)\
			ENUM_CLASS_STRING_ERROR(c_str);\
		RadixTree<E>::const_iterator	it = getTree().find(c_str);\
		if (it != getTree().end())\
			return it->second;\
		ENUM_CLASS_STRING_ERROR(c_str);\
	}\
	static E	from(const std::string& str)\
	{\
		return (from(str.c_str()));\
	}\
private:\
	static const char *unknown() { return ("Unknown"); }

#endif // _ENUMCLASS_H