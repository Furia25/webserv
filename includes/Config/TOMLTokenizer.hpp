/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TOMLTokenizer.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/08 13:01:07 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/08 19:18:08 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _TOMLTOKENIZER_H
# define _TOMLTOKENIZER_H

# include <stdexcept>
# include <stddef.h>
# include <string>
# include <iostream>
# include <deque>
# include <sstream>

# include "TOMLToken.hpp"

namespace TOML
{

class Tokenizer
{
public:
	Tokenizer(std::istream& stream) : buf(stream.rdbuf()), line(1), col(0) {};
	std::deque<Token>&	getTokens();

	void	scan();
protected:
private:
	std::streambuf*		buf;
	std::deque<Token>	tokens;
	size_t				line;
	size_t				col;

	void	addToken(Token::Type type, const std::string& value = "");
	void	scanToken();

	void	error(char c);
	char	consume();
	char	consume(size_t n);
	char	peek();
	char	peekNext();
	bool	match(char c);
	bool	eof();
	void	new_line();

	void	string(bool literal);
	bool	is_multiline(char quote);

	void	handle_escape(std::stringstream& ss, bool multiline);
	void	read_unicode(std::stringstream& ss, size_t n);
	bool	parse_string_content(std::stringstream& ss, char quote, bool multiline, bool literal);
	bool	handle_newline(std::stringstream& ss, char quote, bool multiline, size_t& consecutive_quotes);
	bool	handle_quote(std::stringstream& ss, char quote, bool multiline, size_t& consecutive_quotes);
	void	flush_quotes(std::stringstream& ss, char quote, size_t& consecutive_quotes);
	bool	validate_closing(size_t consecutive_quotes, bool multiline);

	bool	handle_literals(char c);

	int		peek_newline();
};

inline char TOML::Tokenizer::consume(size_t n)
{
	if (n == 0)
        return (0);
	while (n > 1)
	{
		this->consume();
		n--;
	}
	return this->consume();
}

inline char TOML::Tokenizer::consume()
{
	int c = buf->sbumpc();
	col++;
	return c == EOF ? '\0' : static_cast<char>(c);
}

inline char TOML::Tokenizer::peek()
{
	int c = buf->sgetc();
	return c == EOF ? '\0' : static_cast<char>(c);
}

inline char TOML::Tokenizer::peekNext()
{
	this->consume();
	col--;
	char result = this->peek();
	buf->sungetc();
	return (result);
}

inline bool TOML::Tokenizer::eof()
{
	return buf->sgetc() == EOF;
}

inline bool TOML::Tokenizer::match(char c)
{
	if (this->peek() == c)
	{
		this->consume();
		return true;
	}
	return false;
}


} // namespace TOML

#endif // _TOMLTOKENIZER_H