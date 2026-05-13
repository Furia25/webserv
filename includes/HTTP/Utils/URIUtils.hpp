/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   URIUtils.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/13 00:22:32 by antoine           #+#    #+#             */
/*   Updated: 2026/05/13 01:50:18 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef URIUTILS_HPP
# define URIUTILS_HPP

# include <iostream>
# include <vector>

namespace URIUtils
{

static inline std::string	decodeURI(const std::string& input)
{
	std::string result;
	for (size_t i = 0; i < input.length(); ++i) 
	{
		if (input[i] == '%' && i + 2 < input.length()) 
		{
			std::string hexStr = input.substr(i + 1, 2);
			char* end;
			long hexVal = std::strtol(hexStr.c_str(), &end, 16);
			if (*end == '\0') 
			{
				result += static_cast<char>(hexVal);
				i += 2;
			} 
			else 
				result += input[i];
		} 
		else if (input[i] == '+') 
			result += ' ';
		else 
			result += input[i];
	}
	return result;
}

static inline std::string	normalizePath(const std::string& path) 
{
	std::vector<std::string> segments;
	size_t start = 0;
	size_t end = 0;
	while ((end = path.find('/', start)) != std::string::npos) 
	{
		std::string segment = path.substr(start, end - start);
		start = end + 1;
		if (segment.empty() || segment == ".") 
			continue;
		if (segment == "..") 
		{
			if (!segments.empty()) 
				segments.pop_back();
		} 
		else 
			segments.push_back(segment);
	}
	std::string last_segment = path.substr(start);
	if (!last_segment.empty() && last_segment != ".") 
	{
		if (last_segment == "..") 
		{
			if (!segments.empty())
				segments.pop_back();
		} 
		else 
			segments.push_back(last_segment);
	}
	std::string result;
	if (!path.empty() && path[0] == '/') 
		result = "/";
	for (size_t i = 0; i < segments.size(); ++i) 
	{
		result += segments[i];
		if (i < segments.size() - 1) 
			result += "/";
	}
	if (!path.empty() && path[path.length() - 1] == '/' && result != "/") 
		result += "/";
	return result;
}

} // namespace URIUtils

# endif