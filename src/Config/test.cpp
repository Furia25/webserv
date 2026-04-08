/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/08 09:48:28 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/08 18:54:36 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Variant.hpp"
# include "TOMLTokenizer.hpp"

# include <iostream>
# include <fstream>

int main(void)
{
	std::fstream filestream("test.txt");
	TOML::Tokenizer	tokenizer(filestream);
	tokenizer.scan();
	std::cout << "STRING PARSED : " << tokenizer.getTokens().front().getValue() << "\n";
}