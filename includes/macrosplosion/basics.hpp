/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   basics.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 17:53:04 by vdurand           #+#    #+#             */
/*   Updated: 2026/02/05 14:57:40 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _MACROSPLOSION_BASICS_H
# define _MACROSPLOSION_BASICS_H

# include "arithmetics/decrements.hpp"
# include "arithmetics/increments.hpp"

#define M_TRUE	1
#define M_FALSE	0

#define M_EMPTY()
#define M_DEFER(id) id M_EMPTY()
#define M_OBSTRUCT(id) id M_DEFER(M_EMPTY)()
#define M_EXPAND(...) __VA_ARGS__
#define _M_IDENTITY_(x)	x
#define M_IDENTITY(x)	_M_IDENTITY_(x)
#define M_EVAL(...) _M_EVAL1024(__VA_ARGS__)

#define _M_EVAL1024(...) _M_EVAL512(_M_EVAL512(__VA_ARGS__))
#define _M_EVAL512(...) _M_EVAL256(_M_EVAL256(__VA_ARGS__))
#define _M_EVAL256(...) _M_EVAL128(_M_EVAL128(__VA_ARGS__))
#define _M_EVAL128(...) _M_EVAL64(_M_EVAL64(__VA_ARGS__))
#define _M_EVAL64(...) _M_EVAL32(_M_EVAL32(__VA_ARGS__))
#define _M_EVAL32(...) _M_EVAL16(_M_EVAL16(__VA_ARGS__))
#define _M_EVAL16(...) _M_EVAL8(_M_EVAL8(__VA_ARGS__))
#define _M_EVAL8(...) _M_EVAL4(_M_EVAL4(__VA_ARGS__))
#define _M_EVAL4(...) _M_EVAL2(_M_EVAL2(__VA_ARGS__))
#define _M_EVAL2(...) _M_EVAL1(_M_EVAL1(__VA_ARGS__))
#define _M_EVAL1(...) __VA_ARGS__

#define M_PRIMITIVE_CAT(a, ...) a ## __VA_ARGS__
#define M_CAT(a, ...) M_PRIMITIVE_CAT(a, __VA_ARGS__)

#define M_PRIMITIVE_STR(a)	#a
#define M_STR(a)	M_PRIMITIVE_STR(a)

#define _M_IFF_0(x, ...) __VA_ARGS__
#define _M_IFF_1(x, ...) x
#define M_IFF(c) M_PRIMITIVE_CAT(_M_IFF_, c)

#define _M_COMPL_0 1
#define _M_COMPL_1 0
#define M_COMPL(b) M_PRIMITIVE_CAT(_M_COMPL_, b)

#define _M_BITAND_0(y) 0
#define _M_BITAND_1(y) y
#define M_BITAND(x) M_PRIMITIVE_CAT(_M_BITAND_, x)

#define _M_CHECK_N_(x, n, ...) n
#define M_CHECK(...) _M_CHECK_N_(__VA_ARGS__, 0,)

#define M_PROBE(x) x, 1,

#define _M_NOT_0 M_PROBE(~)
#define M_NOT(x) M_CHECK(M_PRIMITIVE_CAT(_M_NOT_, x))

#define M_BOOL(x) M_COMPL(M_NOT(x))
#define M_IF(c) M_IFF(M_BOOL(c))

#define M_HEAD(x, ...)	x
#define M_TAIL(x, ...)	__VA_ARGS__

#define M_EAT(...)
#define M_WHEN(c) M_IF(c)(M_EXPAND, M_EAT)
#define M_ELSE(c) M_IF(c)(M_EAT, M_EXPAND)

#define _M_REPEAT_INDIRECT() _M_REPEAT
#define _M_REPEAT(count, macro, ...) \
	M_WHEN(count) \
	( \
		M_OBSTRUCT(_M_REPEAT_INDIRECT) () \
		( \
			M_DEC(count), macro, __VA_ARGS__ \
		) \
		M_OBSTRUCT(macro) \
		( \
			M_DEC(count), __VA_ARGS__ \
		) \
	)
#define M_REPEAT(count, macro, ...)	M_EVAL(_M_REPEAT(count, macro, __VA_ARGS__))

#define M_COMMA()	,
#define M_MAX_INT	1024
#define M_MIN_INT	0

#define _M_POP_INDIRECT()	_M_POP_
#define _M_POP_(flag, ...) \
	M_ELSE(M_IS_EMPTY(M_TAIL(__VA_ARGS__))) \
	(\
		M_IFF(flag)\
		(\
			M_COMMA() M_HEAD(__VA_ARGS__)\
			,\
			M_HEAD(__VA_ARGS__)\
		)\
		M_OBSTRUCT(_M_POP_INDIRECT)()(1, M_TAIL(__VA_ARGS__))\
	)
#define M_POP(...)	M_EVAL(_M_POP_(0, __VA_ARGS__))

#define _M_FOREACH_INDIRECT()	_M_FOREACH
#define _M_FOREACH(macro, data, ...)	\
	M_ELSE(M_IS_EMPTY(M_HEAD(__VA_ARGS__))) \
	(\
		M_OBSTRUCT(macro)(M_HEAD(__VA_ARGS__), M_TUPLE_REM data)\
		M_OBSTRUCT(_M_FOREACH_INDIRECT)()(macro, data, M_TAIL(__VA_ARGS__))\
	)
#define M_FOREACH(macro, data, ...)	M_EVAL(_M_FOREACH(macro, data, __VA_ARGS__))

#endif // _MACROSPLOSION_BASICS_H
