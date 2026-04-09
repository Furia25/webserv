/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TOMLToken.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/08 13:47:14 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/09 17:51:32 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _TOMLTOKEN_H
# define _TOMLTOKEN_H

# include <stddef.h>
# include <string>

# include "Variant.hpp"

# define TOML_NEW_LINE	'\n'

# define _TOML_TOKEN_SPECIAL_ \
	X(DOUBLE_LBRACKET) \
	X(DOUBLE_RBRACKET) \
	X(COMMENT) \
	X(LBRACKET)\
	X(RBRACKET)\
	X(END_OF_FILE) \
	X(NEW_LINE) \

# define _TOML_TOKEN_OPERATOR_ \
	X(LBRACE, '(') \
	X(RBRACE, ')') \
	X(EQUALS, '=') \
	X(COMMA, ',') \
	X(DOT, '.') \

# define _TOML_TOKEN_LITERALS_ \
	X(BARE_KEY) \
	X(STRING) \
	X(MULTI_STRING) \
	X(LITERAL_STRING) \
	X(MULTI_LITERAL_STRING) \
	X(FLOATING) \
	X(INTEGER) \
	X(BOOLEAN) \
	X(DATETIME) \

# define _TOML_TOKEN_TYPES_ \
	_TOML_TOKEN_SPECIAL_ _TOML_TOKEN_LITERALS_ _TOML_TOKEN_OPERATOR_

namespace TOML
{

class Token
{
public:
	# define X(type, ...)	type,
	enum Type {_TOML_TOKEN_TYPES_};
	# undef X

	Token(Type type, size_t line, size_t col) : type(type), line(line), col(col) {}
	template <typename T>
	Token(Type type, size_t line, size_t col, const T value) : type(type), value(value), line(line), col(col) {}

	Type			getType() const;
	const Variant&	getValue() const;
private:
	Type		type;
	Variant		value;
	size_t		line;
	size_t		col;
};

bool	is_whitespace(char c);
bool	is_control(char c);

} // namespace TOML


#endif // _TOMLTOKEN_H