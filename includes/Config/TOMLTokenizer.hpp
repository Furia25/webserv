/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TOMLTokenizer.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/08 13:01:07 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/14 01:08:11 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _TOMLTOKENIZER_H
# define _TOMLTOKENIZER_

# include <stdexcept>
# include <stddef.h>
# include <string>
# include <iostream>
# include <deque>

# include <sstream>
# include <stdint.h>
# include <cstdio>

# include "TOMLToken.hpp"

namespace toml
{

class Tokenizer
{
public:
	Tokenizer(std::istream& stream) : buf(stream.rdbuf()), line(1), col(0) {};
	Token			next_token();
	const Token&	peek_token();
	bool	eof();
protected:
private:
	std::streambuf		*buf;
	Optional<Token>		actual;
	Optional<Token>		next;
	Token				empty;
	size_t				line;
	size_t				col;

	void	addToken(Token::Type type);
	void	addToken(Token::Type type, const std::string& str);

	void	scanToken();

	void	error(char c);
	char	consume();
	char	consume(size_t n);
	char	peek();
	char	peekNext();
	bool	match(char c);

	void	newLine();

	void	string(bool literal);
	bool	is_multiline(char quote);

	void	lexLiterals();
	void	handleEscape(std::stringstream& ss, bool multiline);
	void	readUnicode(std::stringstream& ss, size_t n);
	bool	parseStringContent(std::stringstream& ss, char quote, bool multiline, bool literal);
	bool	handleNewline(std::stringstream& ss, char quote, bool multiline, size_t& consecutive_quotes);
	bool	handleQuote(std::stringstream& ss, char quote, bool multiline, size_t& consecutive_quotes);
	void	flushQuotes(std::stringstream& ss, char quote, size_t& consecutive_quotes);
	bool	validateClosing(size_t consecutive_quotes, bool multiline);

	int		peek_newline();
};

inline char toml::Tokenizer::consume(size_t n)
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

inline char toml::Tokenizer::consume()
{
	int c = buf->sbumpc();
	col++;
	return c == EOF ? '\0' : static_cast<char>(c);
}

inline char toml::Tokenizer::peek()
{
	int c = buf->sgetc();
	return c == EOF ? '\0' : static_cast<char>(c);
}

inline char toml::Tokenizer::peekNext()
{
	this->consume();
	col--;
	char result = this->peek();
	buf->sungetc();
	return (result);
}

inline bool toml::Tokenizer::eof()
{
	return buf->sgetc() == EOF;
}

inline bool toml::Tokenizer::match(char c)
{
	if (this->peek() == c)
	{
		this->consume();
		return true;
	}
	return false;
}

} // namespace toml

#endif // _TOMLTOKENIZER_H