/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileSystem.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 16:51:11 by antbonin          #+#    #+#             */
/*   Updated: 2026/06/09 19:49:01 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _FILESYSTEM_H
# define _FILESYSTEM_H

# include <iostream>
# include <sys/stat.h>
# include <unistd.h>
# include <dirent.h>
# include <cerrno>
# include <cstring>
# include <cstdio>
# include <stdexcept>
# include <ctime>
# include <sstream>
# include <cstdlib>
# include <sys/time.h>

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

	static inline bool	isExecutable(const std::string& path) 
	{
		return (access(path.c_str(), X_OK) == 0);
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

	static inline void removeFile(const std::string& path)
	{
		if (std::remove(path.c_str()) != 0)
		{
			if (errno == ENOENT)
				return;
			std::string errorMsg = "FileSystem::removeFile failed for '" + path + "': " + std::strerror(errno);
			throw std::runtime_error(errorMsg);
		}
	}

	static inline int removeDirectoryRecursive(const std::string& path)
	{
		DIR *dir = opendir(path.c_str());
		if (!dir)
			return (errno == ENOENT) ? 0 : -1;

		struct dirent	*entry;
		int				result = 0;

		while ((entry = readdir(dir)) != NULL)
		{
			std::string name = entry->d_name;

			if (name == "." || name == "..")
				continue;

			std::string full_path = path + "/" + name;
			struct stat st;

			if (lstat(full_path.c_str(), &st) == -1)
				continue;

			if (S_ISDIR(st.st_mode))
			{
				if (removeDirectoryRecursive(full_path) == -1)
					result = -1;
			}
			else
			{
				if (unlink(full_path.c_str()) == -1)
					result = -1;
			}
		}

		closedir(dir);

		if (rmdir(path.c_str()) == -1)
			result = -1;

		return result;
	}

	static inline std::string	GenerateUniqueFilename(const std::string& baseName);std::string	GenerateUniqueFilename(const std::string& baseName)
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
};

#endif