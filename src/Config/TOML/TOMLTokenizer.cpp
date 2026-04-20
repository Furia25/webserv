/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TOMLTokenizer.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/08 18:39:21 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/19 00:17:35 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config/TOML/TOMLTokenizer.hpp"

toml::Token toml::Tokenizer::next_token()
{
	if (eof() && !this->actual.has_value())
		return Token();
	if (!this->actual.has_value())
		this->scanToken();
	if (!this->next.has_value() && !eof())
		this->scanToken();
	Token temp_token;
	temp_token.swap(*this->actual);
	this->actual.swap(this->next);
	this->next.reset();
	return temp_token;
}

const toml::Token& toml::Tokenizer::peek_token()
{
	if (!this->actual.has_value())
	{
		if (eof())
			return empty;
		this->scanToken();
	}
	return *this->actual;
}

void toml::Tokenizer::scanToken()
{
	while (true)
	{
		char c = this->consume();
		switch (c)
		{
		case '\0': this->addToken(Token::END_OF_FILE); return ;
		case '.': this->addToken(Token::DOT); return ;
		case '{':
			this->addToken(Token::LBRACE);
			return ;
		case '}':
			this->addToken(Token::RBRACE);
			return ;
		case '=':
			this->addToken(Token::EQUALS);
			return ;
		case ',':
			this->addToken(Token::COMMA);
			return ;
		case '[':
			this->addToken(match('[') ? Token::DOUBLE_LBRACKET : Token::LBRACKET);
			return ;
		case ']':
			this->addToken(match(']') ? Token::DOUBLE_RBRACKET : Token::RBRACKET);
			return ;
		
		case '#':
			while (!eof() && !toml::is_control(this->peek()) && peek_newline() == 0)
				this->consume();
			this->addToken(Token::COMMENT);
			return ;
		case '"': this->string(false); return ;
		case '\'': this->string(true); return ;

		case '\t':
		case ' ': continue ;
		case TOML_NEW_LINE:
			this->newLine();
			return ;
		case '\r'	:
			if (match(TOML_NEW_LINE))
				this->newLine();
			return ;

		default:
			this->unget();
			this->lexLiterals();
			return ;
		}
	}
}

void toml::Tokenizer::lexLiterals()
{
	std::stringstream	literal;
	size_t				size = 0;
	size_t				start_col = this->col;
	while (1)
	{
		char c = this->consume();
		if (c == '\0')
			break ;
		if (c == '\r' || c == '\t' || c == ' ' || c == '.' || c == '\''
			|| c == '=' || c == ',' || c == ']' || c == '}' || c == '"'
			|| c == '\n')
		{
			this->unget();
			break;
		}
		literal << c;
		size++;
	}
	this->col = start_col;
	this->addToken(Token::IDENT, literal.str());
	this->col = start_col + size;
}

void toml::Tokenizer::string(bool literal)
{
	static const Token::Type types[2][2] = {
		{ Token::Type::STRING,			Token::Type::MULTI_STRING			},
		{ Token::Type::LITERAL_STRING,	Token::Type::MULTI_LITERAL_STRING	},
	};

	const char			quote	 = literal ? '\'' : '"';
	const bool			multiline = this->is_multiline(quote);
	std::stringstream	ss;

	if (!this->parseStringContent(ss, quote, multiline, literal))
		return ;
	std::string str = ss.str();
	this->addToken(types[literal][multiline], (std::string&)str);
}

bool toml::Tokenizer::is_multiline(char quote)
{
	if (this->peek() != quote || this->peekNext() != quote)
		return false;
	this->consume(2);
	this->consume(this->peek_newline());
	return true;
}

bool toml::Tokenizer::parseStringContent(std::stringstream& ss, char quote, bool multiline, bool literal)
{
	size_t consecutive_quotes = 0;

	while (!this->eof())
	{
		if (this->handleNewline(ss, quote, multiline, consecutive_quotes))
			continue ;
		char c = this->consume();
		if (c == quote)
		{
			if (this->handleQuote(ss, quote, multiline, consecutive_quotes))
				return this->validateClosing(consecutive_quotes, multiline);
			continue ;
		}
		this->flushQuotes(ss, quote, consecutive_quotes);
		if (toml::is_control(c))
		{
			std::stringstream	ss;
			ss << "code" << static_cast<int>(c);
			this->error("Forbidden control character " + ss.str() + " in string");
			continue ;
		}
		if (c == '\\' && !literal)
			this->handleEscape(ss, multiline);
		else
			ss << c;
	}
	return this->validateClosing(consecutive_quotes, multiline);
}

bool toml::Tokenizer::handleNewline(std::stringstream& ss, char quote, bool multiline, size_t& consecutive_quotes)
{
	int nl = this->peek_newline();
	if (nl == 0)
		return false;
	this->flushQuotes(ss, quote, consecutive_quotes);
	this->consume(nl);
	if (multiline)
		ss << '\n';
	else
		this->error("Newline inside a single-line string is not allowed");
	return true;
}

bool toml::Tokenizer::handleQuote(std::stringstream& ss, char quote, bool multiline, size_t& consecutive_quotes)
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

void toml::Tokenizer::flushQuotes(std::stringstream& ss, char quote, size_t& consecutive_quotes)
{
	for (size_t i = 0; i < consecutive_quotes; ++i)
		ss << quote;
	consecutive_quotes = 0;
}

bool toml::Tokenizer::validateClosing(size_t consecutive_quotes, bool multiline)
{
	bool valid = multiline ? consecutive_quotes == 3 : consecutive_quotes == 1;
	if (!valid)
		this->error("Unterminated string, EOF reached before closing quote");
	return valid;
}

void toml::Tokenizer::handleEscape(std::stringstream& ss, bool multiline)
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
	case 'x': this->readUnicode(ss, 2); break;
	case 'u': this->readUnicode(ss, 4); break;
	case 'U': this->readUnicode(ss, 8); break;
	default:
		if (!multiline || (this->peek_newline() == 0 && toml::is_whitespace(escape)))
		{
			this->error("Unable to parse reserved escape sequence (Invalid sequence)");
			break;
		}
		while (!this->eof())
		{
			int nl = this->peek_newline();
			if (nl != 0)
				this->consume(nl);
			else if (toml::is_whitespace(this->peek()))
				this->consume();
			else
				break;
		}
	}
}

void toml::Tokenizer::readUnicode(std::stringstream& ss, size_t n)
{
	uint32_t codepoint = 0;

	for (size_t i = 0; i < n; i++)
	{
		if (this->eof())
		{
			this->error("Unexpected EOF (expected escape sequence)"); // TODO unexpected EOF
			return;
		}
		char c = this->consume();
		codepoint <<= 4;
		if (c >= '0' && c <= '9')		codepoint |= (c - '0');
		else if (c >= 'a' && c <= 'f')	codepoint |= (c - 'a' + 10);
		else if (c >= 'A' && c <= 'F')	codepoint |= (c - 'A' + 10);
		else
		{
			this->error("Invalid hex digit in escape sequence");
			return ;
		}
	}

	if (codepoint <= 0x7F)
		ss << static_cast<char>(codepoint);
	else if (codepoint <= 0x7FF)
	{
		ss << static_cast<char>(0xC0 | (codepoint >> 6));
		ss << static_cast<char>(0x80 | (codepoint & 0x3F));
	}
	else if (codepoint <= 0xFFFF)
	{
		ss << static_cast<char>(0xE0 | (codepoint >> 12));
		ss << static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
		ss << static_cast<char>(0x80 | (codepoint & 0x3F));
	}
	else if (codepoint <= 0x10FFFF)
	{
		ss << static_cast<char>(0xF0 | (codepoint >> 18));
		ss << static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
		ss << static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
		ss << static_cast<char>(0x80 | (codepoint & 0x3F));
	}
	else
		this->error("Codepoint out of valid unicode range");
}

int toml::Tokenizer::peek_newline()
{
	char next = this->peek();
	if (next == TOML_NEW_LINE)
		return (1);
	if (next == '\r' && this->peekNext() == TOML_NEW_LINE)
		return (2);
	return (0);
}

void toml::Tokenizer::error(const std::string& str)
{
	this->error_manager.emitError(str, 1, this->line, this->col);
}

void toml::Tokenizer::newLine()
{
	this->line++;
	this->col = 1;
	this->addToken(Token::NEW_LINE);
	this->error_manager.newSnippet();
}

void toml::Tokenizer::addToken(Token::Type type)
{
	Token token = Token(type, line, col);
	if (this->actual.has_value())
		this->next = token;
	else
		this->actual = token;
}

void toml::Tokenizer::addToken(Token::Type type, const std::string &str)
{
	Token token = Token(type, line, col, str);
	if (this->actual.has_value())
		this->next = token;
	else
		this->actual = token;
}
