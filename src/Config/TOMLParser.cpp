/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TOMLParser.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/10 15:58:56 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/10 21:06:36 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "TOMLParser.hpp"

toml::TOMLParser::TOMLParser(std::istream &stream, toml::Document &document) : state(EXPECT_KEY), tokenizer(stream), document(document)
{
	while (!this->tokenizer.eof())
	{
		Token token = this->tokenizer.next_token();
		Token next = this->tokenizer.peek_token();
		switch (token.getType())
		{
			case Token::COMMENT: break;
			case Token::END_OF_FILE: return ;
			case Token::NEW_LINE: break;
			case Token::EQUALS:
				if (state != EXPECT_KEY)
					goto unexpected_token;
				state = EXPECT_VALUE;
				break;

			case Token::STRING:
				if (state == AFTER_VALUE)
					goto unexpected_token;
				state == EXPECT_VALUE ? this->addValue(token.getLiteral()) : this->handleKeys(token);
				break;
			case Token::MULTI_STRING:
			case Token::LITERAL_STRING:
			case Token::MULTI_LITERAL_STRING:
				if (state != EXPECT_VALUE)
					goto unexpected_token;
				this->addValue(token.getLiteral());
				break;

			case Token::DOUBLE_LBRACKET:
				if (state != EXPECT_KEY)
					goto unexpected_token;
				this->handleHeaders(token);
				break;

			case Token::RBRACKET: this->unest(NEST_BRACE); break;
			case Token::LBRACKET:
				if (state == AFTER_VALUE)
					goto unexpected_token;
				if (state == EXPECT_VALUE)
					this->nest(NEST_BRACE);
				else
					this->handleHeaders(token);
				break;

			case Token::RBRACE: this->unest(NEST_BRACKET); break;
			case Token::LBRACE:
				if (state != EXPECT_VALUE)
					goto unexpected_token;
				state = EXPECT_KEY;
				this->nest(NEST_BRACKET); break;

			case Token::COMMA:
				if (this->nesting.size() == 0 || this->state != AFTER_VALUE)
					goto unexpected_token;
				state = this->nesting.top() == NEST_BRACE ? EXPECT_KEY : EXPECT_VALUE;
				break;

			case Token::IDENT:
				if (state == EXPECT_KEY)
					this->handleKeys(token);
				else
					this->handleLiterals(token);
				break;

			default: unexpected_token:
				this->error('u'); // TODO unexpected token
				break;
		}
	}
	if (this->nesting.size() != 0)
		this->error('u'); // TODO unclosed nested
}

void toml::TOMLParser::nest(NestType type)
{
	this->nesting.push(type);
}

void toml::TOMLParser::unest(NestType type)
{
	if (this->nesting.size() == 0)
		this->error(']'); // "unexpected ]"
	else if (this->nesting.top() != type)
		this->error('['); // TODO expected '}' got ']'
	else
		this->nesting.push(type);
}

void toml::TOMLParser::handleLiterals(const Token &token)
{
}

void toml::TOMLParser::handleKeys(const Token &token)
{
	if (this->validateKey(token.getLiteral()))
		return ;
}

bool toml::TOMLParser::validateKey(const std::string& str)
{
	for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
	{
		switch (*it)
		{
		case '0' ... '9':
		case 'a' ... 'z':
		case 'A' ... 'Z':
		case '_':
		case '-':
			break;
		default:
			this->error(*it); //TODO throw : unexpected char in key token
			return false;
		}
	}
	return true;
}

void toml::TOMLParser::error(char c)
{
}

/*bool toml::Tokenizer::handle_keywords(std::stringstream &literal)
{
	TODO CHANGE THIS TO TRIE
	std::string buf = literal.str();

	if (buf.size() == 4 && buf == "true")
	{
		this->addToken(Token::IDENT, true);
		return true;
	}
	else if (buf.size() == 5 && buf == "false")
	{
		this->addToken(Token::IDENT, false);
		return true;
	}
	return (false);
}*/

/*void toml::Tokenizer::handle_numbers(std::stringstream& literal)
{
	Token::Type			candidate = Token::INTEGER;
	std::ostringstream	ss;
	char				*end_ptr;
	bool				first_char = true;
	int					base = 10;

	while (literal.peek() != EOF && !literal.bad())
	{
		char c = literal.get(); col++;
		char next = literal.peek();
		switch (c)
		{
		case '0':
			if (!first_char)
				goto write;
			switch (next)
			{
			case 'x': base = 16; literal.get(); col++; break;
			case 'o': base = 8;  literal.get(); col++; break;
			case 'b': base = 2;  literal.get(); col++; break;
			case '0' ... '9': this->error('0'); break; // TODO trailing zeros
			}
			break;
		case '.':
			if (first_char || !std::isdigit(next))
				this->error('.'); // TODO unexpected .
			candidate = Token::FLOATING;
			goto write;
			break;
		case '_':
			if (first_char || !std::isdigit(next))
				this->error('_'); // TODO misplaced underscore
			break;
		case 'e':
		case 'E':
		case 'i':
		case 'n':
			if (base == 10)
				candidate = Token::FLOATING;
		default: write:
			first_char = false;
			ss << c;
			break;
		}
	}
	errno = 0;
	if (candidate == Token::INTEGER)
		this->addToken(candidate, std::strtoll(ss.str().c_str(), &end_ptr, base));
	else
		this->addToken(candidate, std::strtod(ss.str().c_str(), &end_ptr));
	if (candidate == Token::FLOATING && base != 10)
		this->error('f'); // TODO base on floats;
	if (errno == ERANGE)
		this->error('r'); // TODO throw overflow
	if (*end_ptr != '\0')
		this->error(*end_ptr); // TODO throw unexpected char
}*/

toml::TOMLParser::~TOMLParser() {}
