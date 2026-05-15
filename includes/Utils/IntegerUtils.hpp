/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IntegerUtils.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/30 13:31:23 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/06 02:40:24 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _INTEGER_UTILS_H
# define _INTEGER_UTILS_H

# include <sstream>
# include <errno.h>
# include <cstdlib>

namespace IntegerUtils
{
	
	static inline unsigned long strtoul_safe(const char *str, int base = 10)
	{
		if (str == NULL)
			throw std::invalid_argument("empty string");
		const char	*ptr = str;
		
		while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r' || *ptr == '\v' || *ptr == '\f')
			++ptr;
		if (*ptr == '\0')
			throw std::invalid_argument("whitespace-only string");
		if (*ptr == '-')
			throw std::invalid_argument("negative value not allowed for unsigned: \"" + std::string(str) + "\"");
		
		char			*end;
		unsigned long	result = std::strtoul(ptr, &end, base);
		errno = 0;
		
		if (errno == ERANGE)
			throw std::out_of_range("value out of unsigned long range: \"" + std::string(str) + "\"");
		if (end == ptr || *end != '\0')
			throw std::invalid_argument("invalid characters in \"" + std::string(str) + "\"");
		return result;
	}
	
	static inline unsigned long strtoul_safe(const std::string& str, int base = 10)
	{
		if (str.empty())
			throw std::invalid_argument("empty string");
		return IntegerUtils::strtoul_safe(str.c_str(), base);
	}
	
	template <typename T>
	static inline std::string itoa(T n)
	{
		std::stringstream ss;
		ss << n;
		return ss.str();
	}
	
};

#endif // _INTEGER_UTILS_H