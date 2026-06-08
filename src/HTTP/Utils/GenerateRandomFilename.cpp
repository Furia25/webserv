/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GenerateRandomFilename.cpp                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/05 17:17:53 by antbonin          #+#    #+#             */
/*   Updated: 2026/06/05 17:29:23 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/Utils/GenerateUniqueFilename.hpp"

std::string	GenerateUniqueFilename(const std::string& baseName)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	std::stringstream ss;
	if (!baseName.empty())
		ss << baseName << "_";
	ss << tv.tv_sec << tv.tv_usec << "_";
	const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	for (int i = 0; i < 3; ++i)
	{
		ss << charset[rand() % (sizeof(charset) - 1)];
	}
	return ss.str();
}
