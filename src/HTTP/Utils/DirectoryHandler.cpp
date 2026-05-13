/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectoryHandler.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/05 16:56:42 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/13 02:09:05 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/Utils/DirectoryHandler.hpp"
# include <dirent.h>

/*Cela a peux etre mettre dnas FileSystem a la place de se faire chier ? jsp*/
size_t	DirectoryHandler::getDirectorySize(const std::string& path) const
{
	size_t	count = 0;
	DIR		*dir = opendir(path.c_str());
	if (dir)
	{
		struct	dirent *ent;
		while((ent = readdir(dir)) != NULL)
			count++;
		closedir(dir);
	}
	return count;
}
