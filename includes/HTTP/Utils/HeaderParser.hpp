/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HeaderParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/21 13:02:54 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/21 21:22:16 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HEADERPARSER_HPP
#define HEADERPARSER_HPP

#include <string>
#include <stdexcept>
#include <cctype>

namespace HeaderParser
{
	inline size_t tryParseHeaderLine(const uint8_t* buffer, size_t size, std::string& key, std::string& value)
	{
		size_t i = 0;
		while (i < size && buffer[i] != '\n')
			i++;
		if (i == size)
			return 0; 
		size_t line_end = i;
		size_t actual_end = (line_end > 0 && buffer[line_end - 1] == '\r') ? line_end - 1 : line_end;
		if (actual_end == 0)
		{
			key.clear();
			value.clear();
			return line_end + 1;
		}
		std::string line(reinterpret_cast<const char*>(buffer), actual_end);
		size_t colon = line.find(':');
		if (colon == std::string::npos)
			throw std::invalid_argument("Malformed header: missing colon");
		key = line.substr(0, colon);
		value = line.substr(colon + 1);
		for (size_t j = 0; j < key.size(); ++j)
			key[j] = std::tolower(key[j]);
		size_t start = value.find_first_not_of(" \t");
		if (start != std::string::npos)
			value = value.substr(start);
		else
			value.clear(); 
		return line_end + 1;
	}
}

#endif // _HEADERPARSER_H