/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/08 09:48:28 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/09 22:29:42 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Variant.hpp"
# include "TOMLTokenizer.hpp"

# include <iostream>
# include <fstream>

# include "macrosplosion.hpp"

int main(void)
{
	std::fstream filestream("test.txt");
	TOML::Tokenizer	tokenizer(filestream);
	tokenizer.scan();
	auto vector = tokenizer.getTokens();
	for (auto it = vector.begin(); it != vector.end(); ++it)
	{
		if (it->getType() == TOML::Token::NEW_LINE)
		{
			std::cout << "\n";
			continue ;
		}
		std::cout << "[";
		switch (it->getType())
		{
			#define X(name, ...) case TOML::Token::name: std::cout << #name; break;
			_TOML_TOKEN_TYPES_
			#undef X
		}
		switch (it->getType())
		{
			case TOML::Token::STRING:
			case TOML::Token::MULTI_STRING:
			case TOML::Token::MULTI_LITERAL_STRING:
			case TOML::Token::LITERAL_STRING:
			case TOML::Token::BARE_KEY: std::cout << " \"" << (*it).getValue().as<std::string>() << "\""; break;
			case TOML::Token::FLOATING: std::cout << " " << (*it).getValue().as<double>(); break;
			case TOML::Token::INTEGER: std::cout << " " << (*it).getValue().as<long long>(); break;
		}
		std::cout << "]";
	}
	std::cout << "\n";
}