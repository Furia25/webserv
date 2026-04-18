/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TOMLToken.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/08 13:47:14 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/19 00:17:25 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _TOMLTOKEN_H
# define _TOMLTOKEN_H

# include <stddef.h>
# include <string>

# include "TOMLVariant.hpp"

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
	X(LBRACE) \
	X(RBRACE) \
	X(EQUALS) \
	X(COMMA) \
	X(DOT) \

# define _TOML_TOKEN_LITERALS_ \
	X(IDENT) \
	X(STRING) \
	X(MULTI_STRING) \
	X(LITERAL_STRING) \
	X(MULTI_LITERAL_STRING) \

# define _TOML_TOKEN_TYPES_ \
	_TOML_TOKEN_SPECIAL_ _TOML_TOKEN_LITERALS_ _TOML_TOKEN_OPERATOR_

namespace toml
{

class Token
{
public:
	# define X(type, ...)	type,
	enum Type {_TOML_TOKEN_TYPES_};
	# undef X

	Token() : type(END_OF_FILE) {};
	Token(Type type, size_t line, size_t col) : type(type), line(line), col(col) {};
	Token(Type type, size_t line, size_t col, const std::string& str) : type(type), literal(str), line(line), col(col) {};
	Token(const Token& other) { *this = other; };

	void	swap(Token& other);
	Token&	operator= (const Token& other);

	const char			*toString() const;
	static const char	*toString(Token::Type type);

	Type				getType() const;
	const std::string&	getLiteral() const;
	size_t				getLine() const { return this->line; };
	size_t				getColumn() const { return this->col; };
private:
	Type		type;
	std::string	literal;
	size_t		line;
	size_t		col;
};

bool	is_whitespace(char c);
bool	is_control(char c);

} // namespace toml


#endif // _TOMLTOKEN_H