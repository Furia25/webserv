/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TOMLParser.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/10 00:12:40 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/10 21:06:45 by vdurand          ###   ########.fr       */
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
	enum NestType	{NEST_BRACE, NEST_BRACKET};
	std::stack<NestType, std::vector<NestType>>	nesting;

	std::vector<std::string>	path;
	toml::Document&				document;
	Tokenizer					tokenizer;

	enum State	{EXPECT_KEY, EXPECT_VALUE, AFTER_VALUE};
	enum State	state;

	template <typename T>
	void	addValue(const T& value);

	void	nest(enum NestType type);
	void	unest(enum NestType type);
	void	handleLiterals(const Token& token);
	void	handleKeys(const Token& token);
	void	handleHeaders(const Token& token);
	bool	validateKey(const std::string& str);

	void	error(char c);
};

}; // namespace toml


#endif // _TOMLPARSER_H