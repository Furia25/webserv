/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileSystem.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antoine <antoine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 16:51:11 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/15 15:07:37 by antoine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


# ifndef _FILESYSTEM_H
# define _FILESYSTEM_H

# include <iostream>
# include <sys/stat.h>
# include <unistd.h>
# include <dirent.h>

namespace FileSystem
{
	static inline bool	exists(const std::string& path) 
	{
		struct stat buffer;
		return (stat(path.c_str(), &buffer) == 0);
	}

	static inline bool	isDirectory(const std::string& path) 
	{
		struct stat buffer;
		if (stat(path.c_str(), &buffer) != 0) return false;
			return S_ISDIR(buffer.st_mode);
	}

	static inline bool	isFile(const std::string& path) 
	{
		struct stat buffer;
		if (stat(path.c_str(), &buffer) != 0) return false;
			return S_ISREG(buffer.st_mode);
	}

	static inline bool	isReadable(const std::string& path) 
	{
		return (access(path.c_str(), R_OK) == 0);
	}

	static inline bool	isWritable(const std::string& path) 
	{
		return (access(path.c_str(), W_OK) == 0);
	}

	static inline size_t	getFileSize(const std::string& path)
	{
		struct stat buffer;
		if (stat(path.c_str(), &buffer) == 0)
			return buffer.st_size;
		return 0;
	}

	static inline std::string	getExtension(const std::string& path)
	{
		size_t pos = path.find_last_of('.');
		if (pos != std::string::npos)
			return path.substr(pos + 1);
		return "";
	}
	
	static inline size_t	getDirectorySize(const std::string& path)
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
};

#endif