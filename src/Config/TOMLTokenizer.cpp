/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TOMLTokenizer.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/08 13:36:10 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/08 17:57:41 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "TOMLTokenizer.hpp"

void TOML::Tokenizer::scan()
{
	while (!eof())
	{
		scanToken();
		this->col++;
	}
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
	std::stringstream	ss("");
	const char quote = literal ? '\'' : '"';

	bool multiline = this->peek() == quote && this->peekNext() == quote;
	if (multiline)
	{
		this->consume(2);
		this->consume(peek_newline());
	}
	size_t	consecutive_quote = 0;
	while (!eof())
	{
		char c = this->consume();
		if (c == quote)
		{
			consecutive_quote++;
			if (!multiline || consecutive_quote == 3)
				break;
		}
		else
		{
			for (size_t i = 0; i < consecutive_quote; i++)
				ss << quote;
			consecutive_quote = 0;
			ss << c;
		}
	}
	if (!multiline || consecutive_quote != 3)
		this->error('d'); /*Malformed String not closed*/
	else
		this->addToken(types[literal][multiline], ss.str());
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
}

void TOML::Tokenizer::new_line()
{
	this->line++;
	this->col = 1;
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
