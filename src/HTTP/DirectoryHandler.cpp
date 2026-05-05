/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectoryHandler.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/05 16:56:42 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/05 17:17:17 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Utils/DirectoryHandler.hpp"
# include <dirent.h>

size_t	DirectoryHandler::getDirectorySize(const std::string& path) const
{
	size_t count = 0;
	DIR *dir = opendir(path.c_str());
	if (dir)
	{
		struct	dirent *ent;
		while((ent = readdir(dir)) != NULL)
			count++;
		closedir(dir);
	}
	return count;
}
