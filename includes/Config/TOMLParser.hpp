/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TOMLParser.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/10 00:12:40 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/18 21:32:18 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _TOMLPARSER_H
# define _TOMLPARSER_H

# include <iostream>
# include <vector>
# include <stack>

# include "TOMLError.hpp"
# include "TOMLTokenizer.hpp"
# include "toml.hpp"

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
		enum Type{BRACE, BRACKET};
		NestContext(enum Type type, toml::Value *node, size_t line, size_t col) : type(type), node(node), line(line), col(col) {};
		Type			type;
		toml::Value		*node;
		size_t			line;
		size_t			col;
	};
	std::stack<NestContext, std::vector<NestContext>>	nesting;

	toml::Value			*current_node;
	toml::Value			*header_node;
	toml::Document&		document;

	TOMLErrorManager	error_manager;
	Tokenizer			tokenizer;

	enum State	{EXPECT_KEY, AFTER_KEY, EXPECT_VALUE, AFTER_VALUE};
	enum State	state;

	template <typename T>
	void	addValue(const T& value);

	void	nest(const Token& token, enum NestContext::Type type);
	void	unest(const Token& token, enum NestContext::Type type);
	void	handleLiterals(Token& token);
	bool	handleKeywords(const std::string& literal);
	void	handleNumbers(const Token& token, std::stringstream& literal);
	void	handleKeys(Token& token);
	void	handleHeaders(Token& token);
	void	handleNewline(Token& token);

	void	handleErrors();

	void	resolveNode(Token& token, bool& previously_created, bool exclude_header);
	void	error(const std::string& str, const Token& token, bool snippet = true);
};

template <typename T>
inline void TOMLParser::addValue(const T& value)
{
	if (this->nesting.size() != 0 && this->nesting.top().type == NestContext::BRACKET)
	{
		toml::Value variant(value);
		variant.setExplicit();
		this->nesting.top().node->as<toml::Array>().push_back(variant);
	}
	else
		*this->current_node = value;
	this->current_node->setExplicit();
	this->state = AFTER_VALUE;
}

}; // namespace toml

#endif // _TOMLPARSER_H