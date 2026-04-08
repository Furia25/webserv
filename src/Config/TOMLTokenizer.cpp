/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TOMLTokenizer.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/08 18:39:21 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/08 19:20:12 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "TOMLTokenizer.hpp"

void TOML::Tokenizer::scan()
{
	while (!eof())
		scanToken();
	this->addToken(Token::END_OF_FILE);
}

void TOML::Tokenizer::scanToken()
{
	char c = this->consume();
	switch (c)
	{
		#define X(name, lexeme) case lexeme: this->addToken(Token::name); break;
		_TOML_TOKEN_OPERATOR_
		#undef X
		case '{'			: this->addToken(match('{') ? Token::DOUBLE_LBRACKET : Token::LBRACKET);
		case '}'			: this->addToken(match('}') ? Token::DOUBLE_RBRACKET : Token::RBRACKET);
		case '\0'			: this->addToken(Token::END_OF_FILE); break;
		case TOML_NEW_LINE	: this->new_line(); break;
		case '\r'	:
			if (match(TOML_NEW_LINE))
				this->new_line();
		case '#'	:
			while (!TOML::is_control(this->peek()) || peek_newline() == 0 || eof())
				this->consume();
			this->addToken(Token::COMMENT);
			break;
		case '"'	: this->string(false); break;
		case '\''	: this->string(true); break;
	default:
		if (!this->handle_literals(c))
			this->error(c); /*UNKNOWN THING*/
		break;
	}
}

void TOML::Tokenizer::string(bool literal)
{
	static const Token::Type types[2][2] = {
		{ Token::Type::STRING,			Token::Type::MULTI_STRING			},
		{ Token::Type::LITERAL_STRING,	Token::Type::MULTI_LITERAL_STRING	},
	};

	const char			quote	 = literal ? '\'' : '"';
	const bool			multiline = this->is_multiline(quote);
	std::stringstream	ss;

	if (!this->parse_string_content(ss, quote, multiline, literal))
		return ;
	this->addToken(types[literal][multiline], ss.str());
}

bool TOML::Tokenizer::is_multiline(char quote)
{
	if (this->peek() != quote || this->peekNext() != quote)
		return false;
	this->consume(2);
	this->consume(this->peek_newline());
	return true;
}

bool TOML::Tokenizer::parse_string_content(std::stringstream& ss, char quote, bool multiline, bool literal)
{
	size_t consecutive_quotes = 0;

	while (!this->eof())
	{
		if (this->handle_newline(ss, quote, multiline, consecutive_quotes))
			continue ;
		char c = this->consume();
		if (c == quote)
		{
			if (this->handle_quote(ss, quote, multiline, consecutive_quotes))
				return this->validate_closing(consecutive_quotes, multiline);
			continue ;
		}
		this->flush_quotes(ss, quote, consecutive_quotes);
		if (TOML::is_control(c))
		{
			this->error('d'); // throw : forbidden control character U+0000-U+0008, U+000A-U+001F, U+007F in string
			continue ;
		}
		if (c == '\\' && !literal)
			this->handle_escape(ss, multiline);
		else
			ss << c;
	}
	return this->validate_closing(consecutive_quotes, multiline);
}

bool TOML::Tokenizer::handle_newline(std::stringstream& ss, char quote, bool multiline, size_t& consecutive_quotes)
{
	int nl = this->peek_newline();
	if (nl == 0)
		return false;
	this->flush_quotes(ss, quote, consecutive_quotes);
	this->consume(nl);
	if (multiline)
		ss << '\n';
	else
		this->error('d'); // throw : newline inside a single-line string is not allowed;
	return true;
}

bool TOML::Tokenizer::handle_quote(std::stringstream& ss, char quote, bool multiline, size_t& consecutive_quotes)
{
	consecutive_quotes++;
	if (!multiline)
		return true;
	if (consecutive_quotes < 3)
		return false;
	size_t bonus = 0;
	while (bonus < 2 && !this->eof() && this->peek() == quote)
	{
		this->consume();
		bonus++;
	}
	for (size_t i = 0; i < bonus; ++i)
		ss << quote;
	return true;
}

void TOML::Tokenizer::flush_quotes(std::stringstream& ss, char quote, size_t& consecutive_quotes)
{
	for (size_t i = 0; i < consecutive_quotes; ++i)
		ss << quote;
	consecutive_quotes = 0;
}

bool TOML::Tokenizer::validate_closing(size_t consecutive_quotes, bool multiline)
{
	bool valid = multiline ? consecutive_quotes == 3 : consecutive_quotes == 1;
	if (!valid)
		this->error('d'); // throw : unterminated string, EOF reached before closing quote
	return valid;
}

bool TOML::Tokenizer::handle_literals(char c)
{
	return false;
}

void TOML::Tokenizer::handle_escape(std::stringstream& ss, bool multiline)
{
	if (this->eof())
		return ;
	char escape = this->consume();
	switch (escape)
	{
	case 'b': ss << '\b'; break;
	case 't': ss << '\t'; break;
	case 'n': ss << '\n'; break;
	case 'f': ss << '\f'; break;
	case 'r': ss << '\r'; break;
	case 'e': ss << '\e'; break;
	case '\"': ss << '\"'; break;
	case '\\': ss << '\\'; break;
	case 'x': this->read_unicode(ss, 2); break;
	case 'u': this->read_unicode(ss, 4); break;
	case 'U': this->read_unicode(ss, 8); break;
	default:
		if (!multiline || (this->peek_newline() == 0 && TOML::is_whitespace(escape)))
		{
			this->error('d'); /*TODO Reserved escape sequence*/
			break ;
		}
		while (!this->eof())
		{
			int nl = this->peek_newline();
			if (nl != 0)
				this->consume(nl);
			else if (TOML::is_whitespace(this->peek()))
				this->consume();
			else
				break ;
		}
	}
}

void TOML::Tokenizer::read_unicode(std::stringstream& ss, size_t n)
{
	uint32_t codepoint = 0;

	for (int i = 0; i < n; i++)
	{
		if (this->eof())
		{
			this->error('u'); // TODO unexpected EOF
			return;
		}
		char c = this->consume();
		codepoint <<= 4;
		if (c >= '0' && c <= '9')		codepoint |= (c - '0');
		else if (c >= 'a' && c <= 'f')	codepoint |= (c - 'a' + 10);
		else if (c >= 'A' && c <= 'F')	codepoint |= (c - 'A' + 10);
		else
		{
			this->error('u'); //TODO invalid hex digit
			return ;
		}
	}

	if (codepoint <= 0x7F)
		ss << (char)(codepoint);
	else if (codepoint <= 0x7FF)
	{
		ss << (char)(0xC0 | (codepoint >> 6));
		ss << (char)(0x80 | (codepoint & 0x3F));
	}
	else if (codepoint <= 0xFFFF)
	{
		ss << (char)(0xE0 | (codepoint >> 12));
		ss << (char)(0x80 | ((codepoint >> 6) & 0x3F));
		ss << (char)(0x80 | (codepoint & 0x3F));
	}
	else if (codepoint <= 0x10FFFF)
	{
		ss << (char)(0xF0 | (codepoint >> 18));
		ss << (char)(0x80 | ((codepoint >> 12) & 0x3F));
		ss << (char)(0x80 | ((codepoint >> 6) & 0x3F));
		ss << (char)(0x80 | (codepoint & 0x3F));
	}
	else
		this->error('u'); // codepoint out of valid Unicode range
}

int TOML::Tokenizer::peek_newline()
{
	char next = this->peek();
	if (next == TOML_NEW_LINE)
		return (1);
	if (next == '\r' && this->peekNext() == TOML_NEW_LINE)
		return (2);
	return (0); 
}

void TOML::Tokenizer::error(char c)
{
	std::cout << "ERROR: Line: " << line << " Col: " << col << " char: " << c << "\n";
}

void TOML::Tokenizer::new_line()
{
	this->line++;
	this->col = 0;
	this->addToken(Token::NEW_LINE);
}

void TOML::Tokenizer::addToken(Token::Type type, const std::string& value)
{
	this->tokens.push_back(Token(type, this->line, this->col, value));
}


std::deque<TOML::Token>& TOML::Tokenizer::getTokens()
{
	return this->tokens;
}
