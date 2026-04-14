/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TOMLParser.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/10 15:58:56 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/14 17:49:29 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "TOMLParser.hpp"

toml::TOMLParser::TOMLParser(std::istream &stream, toml::Document &document) : state(EXPECT_KEY), tokenizer(stream),
	document(document), header_node(&document.getRoot()), current_node(&document.getRoot())
{
	while (1)
	{
		Token token = this->tokenizer.next_token();
		switch (token.getType())
		{
			case Token::COMMENT: break;
			case Token::END_OF_FILE:
				this->handleNewline(token);
				goto exit;
			case Token::NEW_LINE:
				this->handleNewline(token);
				break;

			case Token::EQUALS:
				if (state != AFTER_KEY)
					goto unexpected_token;
				state = EXPECT_VALUE;
				break;

			case Token::STRING:
			case Token::LITERAL_STRING:
				if (state == AFTER_VALUE)
					goto unexpected_token;
				if (state == EXPECT_VALUE)
					this->addValue(token.getLiteral());
				else
					this->handleKeys(token);
				break;
			case Token::MULTI_STRING:
			case Token::MULTI_LITERAL_STRING:
				if (state != EXPECT_VALUE)
					goto unexpected_token;
				this->addValue(token.getLiteral());
				break;

			case Token::DOUBLE_LBRACKET:
				if (state == AFTER_VALUE)
					goto unexpected_token;
				else if (state == EXPECT_VALUE)
				{
					this->nest(NestContext::BRACKET);
					this->nest(NestContext::BRACKET);
				}
				else
					this->handleHeaders(token);
				break;

			case Token::DOUBLE_RBRACKET:
					this->unest(NestContext::BRACKET);
					this->unest(NestContext::BRACKET);
				break;

			case Token::RBRACKET:
				if (state != EXPECT_KEY)
					this->unest(NestContext::BRACKET);
				break;
			case Token::LBRACKET:
				if (state == AFTER_VALUE)
					goto unexpected_token;
				if (state == EXPECT_VALUE)
					this->nest(NestContext::BRACKET);
				else
					this->handleHeaders(token);
				break;

			case Token::RBRACE: this->unest(NestContext::BRACE); break;
			case Token::LBRACE:
				if (state != EXPECT_VALUE)
					goto unexpected_token;
				this->nest(NestContext::BRACE);
				break;

			case Token::COMMA:
				if (this->nesting.size() == 0 || this->state != AFTER_VALUE)
					goto unexpected_token;
				state = this->nesting.top().type == NestContext::BRACE ? EXPECT_KEY : EXPECT_VALUE;
				break;

			case Token::IDENT:
				if (state == EXPECT_KEY)
					this->handleKeys(token);
				else
					this->handleLiterals(token);
				break;

			default: unexpected_token:
				this->error("Unexpected Token", token.getType());
				break;
		}
	}
	exit:
	if (this->nesting.size() != 0)
		this->error("unclosed nested", Token::RBRACE);
}

void toml::TOMLParser::nest(NestContext::Type type)
{
	Variant node = (type == NestContext::BRACKET ? toml::Value(toml::Array()) : toml::Value(toml::Table()));
	if (this->nesting.size() > 0 && this->nesting.top().type == NestContext::BRACKET)
	{
		toml::Array& array = this->nesting.top().node->as<toml::Array>();
		array.push_back(node);
		this->current_node = &array.back();
	}
	else
		*this->current_node = node;
	this->nesting.push(NestContext(type, this->current_node));
	this->state = (type == NestContext::BRACE) ? EXPECT_KEY : EXPECT_VALUE;
}

void toml::TOMLParser::unest(NestContext::Type type)
{
	if (this->nesting.size() == 0)
		return this->error("unexpected token", Token::LBRACKET);
	if (this->nesting.top().type != type)
		return this->error("unexpected token", Token::LBRACKET);

	this->nesting.pop();
	if (this->nesting.size() > 0)
		this->current_node = this->nesting.top().node;
	else
		this->current_node = &this->document.getRoot();
	this->state = AFTER_VALUE;
}

void toml::TOMLParser::resolveNode(Token& token, bool& previously_created)
{
	bool	last;
	do
	{
		#define ERROR(x, y) do {this->error(x, y); return ;} while (false)
		Token::Type	token_type = token.getType();
		if (token_type != Token::IDENT && token_type != Token::STRING && token_type != Token::LITERAL_STRING)
			ERROR("unexpected token", token_type);
		if (token_type == Token::IDENT && !this->validateKey(token.getLiteral()))
			return;
		if (current_node->getType() == toml::Value::ARRAY)
		{
			Array& arr = current_node->as<Array>();
			if (arr.empty())
				ERROR("redefine array", token_type);
			current_node = &arr.back();
		}
		if (this->current_node != header_node &&
				(this->current_node->isExplicit() || current_node->getType() != toml::Value::TABLE))
			ERROR("redefine table", token_type);

		toml::Table& current_table = current_node->as<toml::Table>();
		previously_created = current_table.contain(token.getLiteral());
		last = this->tokenizer.peek_token().getType() != Token::DOT;
		if (!previously_created)
			current_table.insert(token.getLiteral(), last ? Value() : Value(Table()));
		current_node = &current_table.find(token.getLiteral())->second;
		if (!last)
		{
			this->tokenizer.next_token();
			token = this->tokenizer.next_token();
		}
	} while (!last);
}

void toml::TOMLParser::handleKeys(Token& token)
{
	this->state = AFTER_KEY;
	this->current_node = header_node;
	if (this->nesting.size() != 0)
		this->current_node = this->nesting.top().node;
	bool previously_created = false;
	this->resolveNode(token, previously_created);
	if (previously_created)
	{
		this->error("Cant redefine key", Token::IDENT);
		return ;
	}
}

void toml::TOMLParser::handleHeaders(Token& token)
{
	this->current_node = &this->document.getRoot();
	bool table_array = token.getType() == Token::DOUBLE_LBRACKET;
	token = this->tokenizer.next_token();
	bool previously_created = false;
	this->resolveNode(token, previously_created);
	if (current_node->isExplicit() && 
		(!table_array || current_node->as<Array>().empty() || current_node->as<Array>().front().isExplicit()))
		return this->error("Redefine header", token.getType());
	token = this->tokenizer.next_token();
	if (table_array ? token.getType() != Token::DOUBLE_RBRACKET : token.getType() != Token::RBRACKET)
		return this->error("Unclosed header", token.getType());
	if (!previously_created)
		*this->current_node = table_array ? Value(Array()) : Value(Table());
	this->current_node->setExplicit();
	if (table_array)
	{
		Array& array = this->current_node->as<Array>();
		array.push_back(Variant(Table()));
		this->current_node = &array.back();
	}
	this->header_node = this->current_node;
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
			this->error("unexpected char in key token", Token::IDENT);
			return false;
		}
	}
	return true;
}

void toml::TOMLParser::handleNewline(Token& token)
{
	if (state == EXPECT_VALUE)
	{
		if (this->nesting.empty() || this->nesting.top().type != NestContext::BRACKET)
			this->error("Value expected before newline", token.getType());
	}
	else if (state == AFTER_KEY)
	{
		this->error("Key ended without value ", token.getType());
	}
	if (!this->nesting.empty() && this->nesting.top().type == NestContext::BRACKET)
		state = EXPECT_VALUE;
	else
		state = EXPECT_KEY;
}

void toml::TOMLParser::error(const char *str, Token::Type type)
{
	throw std::runtime_error(std::string("Error: ") + str + " " + Token::toString(type));
}

void toml::TOMLParser::handleLiterals(Token& token)
{
	if (!handleKeywords(token.getLiteral()))
	{
		std::stringstream	ss;
		ss << token.getLiteral();
		
		while (this->tokenizer.peek_token().getType() == Token::DOT)
		{
			token = this->tokenizer.next_token();
			ss << '.';
			if (this->tokenizer.peek_token().getType() != Token::IDENT)
				break;
			token = this->tokenizer.next_token();
			ss << token.getLiteral();
		}
		return handleNumbers(ss);
	}
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

void toml::TOMLParser::handleNumbers(std::stringstream& literal)
{
	Value::Type			candidate = Value::INTEGER;
	std::ostringstream	ss;
	char				*end_ptr;
	bool				first_char = true;
	int					base = 10;
	char                c, next;

	while (literal.get(c))
	{
		next = literal.peek();
		if (literal.eof())
			next = '\0';
		switch (c)
		{
		case '0':
			if (!first_char)
				goto write;
			switch (next)
			{
			case 'x': base = 16; literal.get(); break;
			case 'o': base = 8;  literal.get(); break;
			case 'b': base = 2;  literal.get(); break;
			default:
				if (std::isdigit(next))
					this->error("leading zeros", Token::IDENT);
				break;
			}
			break;
		case '.':
			if (first_char || !std::isdigit(next))
				this->error("unexpected dot", Token::IDENT);
			candidate = Value::FLOATING;
			goto write;
		case '_':
			if (first_char || !std::isdigit(next))
				this->error("unexpected underscore", Token::IDENT);
			break;
		case 'e':
		case 'E':
		case 'i': // inf
		case 'n': // nan
			if (base == 10)
				candidate = Value::FLOATING;
		default: write:
			first_char = false;
			ss << c;
			break;
		}
	}

	errno = 0;
	if (candidate == Value::FLOATING && base != 10)
		this->error("base in float", Token::IDENT);
	if (candidate == Value::INTEGER)
		this->addValue(std::strtoll(ss.str().c_str(), &end_ptr, base));
	else
		this->addValue(std::strtod(ss.str().c_str(), &end_ptr));
	if (errno == ERANGE)
		this->error("overflow", Token::IDENT);
	if (*end_ptr != '\0')
		this->error("unexpected char", Token::IDENT);
}

toml::TOMLParser::~TOMLParser() {}
