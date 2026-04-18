/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TOMLParser.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/10 15:58:56 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/19 00:11:00 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "TOMLParser.hpp"

toml::TOMLParser::TOMLParser(std::istream &stream, toml::Document &document) : state(EXPECT_KEY), tokenizer(stream, this->error_manager),
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
					this->nest(token, NestContext::BRACKET);
					this->nest(token, NestContext::BRACKET);
				}
				else
					this->handleHeaders(token);
				break;

			case Token::DOUBLE_RBRACKET:
					this->unest(token, NestContext::BRACKET);
					this->unest(token, NestContext::BRACKET);
				break;

			case Token::RBRACKET:
				if (state != EXPECT_KEY)
					this->unest(token, NestContext::BRACKET);
				break;
			case Token::LBRACKET:
				if (state == AFTER_VALUE)
					goto unexpected_token;
				if (state == EXPECT_VALUE)
					this->nest(token, NestContext::BRACKET);
				else
					this->handleHeaders(token);
				break;

			case Token::RBRACE: this->unest(token, NestContext::BRACE); break;
			case Token::LBRACE:
				if (state != EXPECT_VALUE)
					goto unexpected_token;
				this->nest(token, NestContext::BRACE);
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
				this->error("Unexpected token", token);
				break;
		}
	}
	exit:
	handleErrors();
}

void toml::TOMLParser::nest(const Token& token, NestContext::Type type)
{
	Variant node = (type == NestContext::BRACKET ? toml::Value(toml::Array()) : toml::Value(toml::Table()));
	node.setExplicit();
	if (this->nesting.size() > 0 && this->nesting.top().type == NestContext::BRACKET)
	{
		toml::Array& array = this->nesting.top().node->as<toml::Array>();
		array.push_back(node);
		this->current_node = &array.back();
	}
	else
		*this->current_node = node;
	this->nesting.push(NestContext(type, this->current_node, token.getLine(), token.getColumn()));
	this->state = (type == NestContext::BRACE) ? EXPECT_KEY : EXPECT_VALUE;
}

void toml::TOMLParser::unest(const Token& token, NestContext::Type type)
{
	if (this->nesting.size() == 0)
		return this->error("unexpected token", token);
	if (this->nesting.top().type != type)
		return this->error("unexpected token", token);

	this->nesting.pop();
	if (this->nesting.size() > 0)
		this->current_node = this->nesting.top().node;
	else
		this->current_node = &this->document.getRoot();
	this->state = AFTER_VALUE;
}

void toml::TOMLParser::handleKeys(Token& token)
{
	this->state = AFTER_KEY;
	this->current_node = header_node;
	if (this->nesting.size() != 0)
		this->current_node = this->nesting.top().node;
	bool previously_created = false;
	this->resolveNode(token, previously_created, true);
	if (previously_created)
	{
		this->error("Key already defined, can't be redefined", token);
		return ;
	}
}

static bool validateKey(const std::string& str);

void toml::TOMLParser::resolveNode(Token& token, bool& previously_created, bool exclude_header)
{
	bool	last;
	do
	{
		#define ERROR(x, y) do {this->error(x, y); return ;} while (false)
		Token::Type	token_type = token.getType();
		if (token_type != Token::IDENT && token_type != Token::STRING && token_type != Token::LITERAL_STRING)
			ERROR("Invalid Token as key", token);
		if (token_type == Token::IDENT && !validateKey(token.getLiteral()))
			ERROR("Invalid character in key", token);
		if (current_node->getType() == toml::Value::ARRAY)
		{
			Array& arr = current_node->as<Array>();
			if (arr.empty())
				throw std::runtime_error("Fatal Error in toml parsing, array should not be empty");
			current_node = &arr.back();
		}
		toml::Table& current_table = current_node->as<toml::Table>();
		previously_created = current_table.contain(token.getLiteral());
		last = this->tokenizer.peek_token().getType() != Token::DOT;
		if (!previously_created)
			current_table.insert(token.getLiteral(), last ? Value() : Value(Table()));
		current_node = &current_table.find(token.getLiteral())->second;
		if (this->nesting.size() == 0 || this->current_node != this->nesting.top().node)
		{
			if (this->current_node->isExplicit() || (exclude_header && this->current_node->isHeader()))
				ERROR("Key already defined, tables can't be redefined", token);
		}
		if (!last)
		{
			this->tokenizer.next_token();
			token = this->tokenizer.next_token();
		}
	} while (!last);
}

void toml::TOMLParser::handleHeaders(Token& token)
{
	this->current_node = &this->document.getRoot();
	bool table_array = token.getType() == Token::DOUBLE_LBRACKET;
	token = this->tokenizer.next_token();
	bool previously_created = false;
	this->resolveNode(token, previously_created, false);
	if (current_node->isHeader() && !table_array)
		return this->error("Key already defined, headers can't be redefined", token);
	token = this->tokenizer.next_token();
	if (table_array ? token.getType() != Token::DOUBLE_RBRACKET : token.getType() != Token::RBRACKET)
		return this->error("Unclosed header", token);
	if (!previously_created)
		*this->current_node = table_array ? Value(Array()) : Value(Table());
	this->current_node->setHeader();
	if (table_array)
	{
		Array& array = this->current_node->as<Array>();
		array.push_back(Variant(Table()));
		this->current_node = &array.back();
	}
	this->header_node = this->current_node;
	this->state = AFTER_VALUE;
}

static bool validateKey(const std::string& str)
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
			this->error("Value expected before newline", token);
	}
	else if (state == AFTER_KEY)
	{
		this->error("Key ended without value ", token);
	}
	if (!this->nesting.empty() && this->nesting.top().type == NestContext::BRACKET)
		state = EXPECT_VALUE;
	else
		state = EXPECT_KEY;
}

void toml::TOMLParser::handleErrors()
{
	if (this->nesting.size() != 0)
	{
		NestContext&	context = this->nesting.top();
		Token::Type		type = context.type == NestContext::BRACE ? Token::RBRACE : Token::LBRACE;
		Token	temp_token(type, context.line, context.col);
		this->error(std::string("Unclosed scope ") + (type == Token::RBRACE ? '[' : '{'), temp_token, false);
	}
	if (this->error_manager.hasErrors())
		throw this->error_manager.getError();
}

void toml::TOMLParser::error(const std::string& str, const Token& token, bool snippet)
{
	this->error_manager.emitError(str, token.getLiteral().size(), token.getLine(), token.getColumn(), snippet);
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
		return handleNumbers(token, ss);
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

#include <ios>

void toml::TOMLParser::handleNumbers(const Token& token, std::stringstream& literal)
{
	Value::Type			candidate = Value::INTEGER;
	std::ostringstream	ss;
	char				*end_ptr;
	bool				first_char = true;
	int					base = 10;
	char				c, next;

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
			case 'o': base = 8; literal.get(); break;
			case 'b': base = 2; literal.get(); break;
			default:
				if (std::isdigit(next))
					this->error("Unexpected leading zeros", token);
				goto write;
				break;
			}
			break;
		case '.':
			if (first_char || !std::isdigit(next))
				this->error("Unexpected dot", token);
			candidate = Value::FLOATING;
			goto write;
		case '_':
			if (first_char || !std::isalnum(next))
				this->error("Unexpected underscore", token);
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

	if (first_char)
		this->error("Empty Number", token);
	if (candidate == Value::FLOATING && base != 10)
		this->error("Base sequence in float", token);
	errno = 0;
	std::string temp_string = ss.str();
	if (candidate == Value::INTEGER)
		this->addValue(std::strtoll(temp_string.c_str(), &end_ptr, base));
	else
		this->addValue(std::strtod(temp_string.c_str(), &end_ptr));
	if (errno == ERANGE)
		this->error("Numeric overflow", token);
	if (errno == 0 && end_ptr && *end_ptr != '\0')
		this->error("Unexpected char in number", token);
}

toml::TOMLParser::~TOMLParser() {}
