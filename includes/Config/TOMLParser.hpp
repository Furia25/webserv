/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TOMLParser.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/10 00:12:40 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/14 00:43:55 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _TOMLPARSER_H
# define _TOMLPARSER_H

# include <iostream>
# include <vector>
# include <stack>

# include "TOMLTokenizer.hpp"
# include "TOMLDocument.hpp"

namespace toml
{

class TOMLParser
{
public:
	TOMLParser(std::istream& stream, toml::Document& document);
	~TOMLParser();

private:
	struct NestContext
	{
		enum Type{NEST_BRACE, NEST_BRACKET};
		NestContext(enum Type type, toml::Value *node) : type(type), node(node) {};
		Type			type;
		toml::Value		*node;
	};
	std::stack<NestContext, std::vector<NestContext>>	nesting;

	toml::Value			*current_node;
	toml::Document&		document;
	Tokenizer			tokenizer;

	enum State	{EXPECT_KEY, EXPECT_VALUE, AFTER_VALUE};
	enum State	state;

	template <typename T>
	void	addValue(const T& value);

	void	nest(enum NestContext::Type type);
	void	unest(enum NestContext::Type type);
	void	handleLiterals(Token& token);
	bool	handleKeywords(const std::string& literal);
	void	handleNumbers(const std::string& literal);
	void	handleKeys(Token& token);
	void	handleHeaders(Token& token);
	bool	validateKey(const std::string& str);

	void	resolveNode(Token& token, bool& previously_created);
	void	error(char c);
};

template <typename T>
inline void TOMLParser::addValue(const T& value)
{
	if (this->nesting.size() != 0 && this->nesting.top().type == NestContext::NEST_BRACE)
	{
		if (this->current_node->getType() != Value::ARRAY)
			this->current_node = Array();
		this->current_node.as<toml::Array>().push_back(Variant(value));
	}
	else
		this->current_node = value;
	this->current_node->setExplicit = true;
}

}; // namespace toml

#endif // _TOMLPARSER_H