/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TOMLParser.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/10 15:58:56 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/14 00:53:05 by vdurand          ###   ########.fr       */
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
			case Token::NEW_LINE: 
				if (state != AFTER_VALUE)
					this->error('n'); // TODO Unspecified values are invalid.
				break;
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

			case Token::RBRACKET: this->unest(NestContext::NEST_BRACKET); break;
			case Token::LBRACKET:
				if (state == AFTER_VALUE)
					goto unexpected_token;
				if (state == EXPECT_VALUE)
					this->nest(NestContext::NEST_BRACKET);
				else
					this->handleHeaders(token);
				break;

			case Token::RBRACE: this->unest(NestContext::NEST_BRACE); break;
			case Token::LBRACE:
				if (state != EXPECT_VALUE)
					goto unexpected_token;
				state = EXPECT_KEY;
				this->nest(NestContext::NEST_BRACE); break;

			case Token::COMMA:
				if (this->nesting.size() == 0 || this->state != AFTER_VALUE)
					goto unexpected_token;
				state = this->nesting.top().type == NestContext::NEST_BRACE ? EXPECT_KEY : EXPECT_VALUE;
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

void toml::TOMLParser::nest(NestContext::Type type)
{
	this->nesting.push(NestContext(type, current_node));
}

void toml::TOMLParser::unest(NestContext::Type type)
{
	if (this->nesting.size() == 0)
		this->error(']'); // TODO "unexpected ]"
	else if (this->nesting.top().type != type)
		this->error('['); // TODO expected '}' got ']'
	else
		this->nesting.pop();
}

void toml::TOMLParser::resolveNode(Token& token, bool& previously_created)
{
	bool	last;
	do
	{
		#define ERROR(x) do {this->error(x); return ;} while (false)

		if (token.getType() != Token::IDENT)
			ERROR('u'); // TODO "unexpected token"
		if (this->current_node->isExplicit() || current_node->getType() != toml::Value::TABLE)
			ERROR('u'); // TODO explicit key
		if (!this->validateKey(token.getLiteral()))
			return ;

		toml::Table& current_table = current_node->as<toml::Table>();
		previously_created = current_table.contain(token.getLiteral());
		last = this->tokenizer.peek_token().getType() != Token::DOT;
		if (!previously_created)
			current_table.insert(token.getLiteral(), last ? Value() : Value(Table()));
		current_node = &current_table.find(token.getLiteral())->second;
		this->tokenizer.next_token();
		token = this->tokenizer.next_token();
	} while (!last);
}

void toml::TOMLParser::handleKeys(Token& token)
{
	if (this->nesting.size() != 0)
		this->current_node = this->nesting.top().node;
	else
		this->current_node = &this->document.getRoot();
	bool previously_created;
	this->resolveNode(token, previously_created);
	if (previously_created)
	{
		this->error('e'); //TODO Cant redefine key
		return ;
	}
	this->current_node->setExplicit();
	if (token.getType() != Token::EQUALS)
		this->error('e'); //TODO unexpected token i want an equal
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

void toml::TOMLParser::handleLiterals(Token& token)
{
	if (!handleKeywords(token.getLiteral()))
		handleNumbers(token.getLiteral());
}

bool toml::TOMLParser::handleKeywords(const std::string& literal)
{
	if (literal.size() == 4 && literal == "true")
	{
		this->addValue((bool)true);
		return true;
	}
	else if (literal.size() == 5 && literal == "false")
	{
		this->addValue((bool)false);
		return true;
	}
	return (false);
}

void toml::TOMLParser::handleNumbers(const std::string& literal)
{
	Value::Type			candidate = Value::INTEGER;
	std::ostringstream	ss;
	char				*end_ptr;
	bool				first_char = true;
	int					base = 10;

	for (std::string::const_iterator it = literal.begin(); it != literal.end(); ++it)
	{
		char c = *it;
		char next = (std::next(it) != literal.end()) ? *std::next(it) : '\0';

		switch (c)
		{
		case '0':
			if (!first_char)
				goto write;
			switch (next)
			{
			case 'x': base = 16; ++it; break;
			case 'o': base = 8;  ++it; break;
			case 'b': base = 2;  ++it; break;
			default:
				if (std::isdigit((unsigned char)next))
					this->error('0'); // TODO leading zeros
				break;
			}
			break;
		case '.':
			if (first_char || !std::isdigit((unsigned char)next))
				this->error('.');
			candidate = Value::FLOATING;
			goto write;
		case '_':
			if (first_char || !std::isdigit((unsigned char)next))
				this->error('_');
			break;
		case 'e':
		case 'E':
		case 'i': // inf
		case 'n': // nan
			if (base == 10)
				candidate = Value::FLOATING;
			// fallthrough
		default: write:
			first_char = false;
			ss << c;
			break;
		}
	}

	errno = 0;
	if (candidate == Value::FLOATING && base != 10)
		this->error('f'); // TODO base in float
	if (candidate == Value::INTEGER)
		this->addValue(std::strtoll(ss.str().c_str(), &end_ptr, base));
	else
		this->addValue(std::strtod(ss.str().c_str(), &end_ptr));
	if (errno == ERANGE)
		this->error('r'); // TODO overflow
	if (*end_ptr != '\0')
		this->error(*end_ptr); // TODO unexpected char
}

toml::TOMLParser::~TOMLParser() {}
