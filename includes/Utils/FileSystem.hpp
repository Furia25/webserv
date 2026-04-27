/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileSystem.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 16:51:11 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/27 14:23:39 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


# ifndef _FILESYSTEM_H
# define _FILESYSTEM_H

# include <iostream>
# include <sys/stat.h>
# include <unistd.h>

namespace FileSystem
{
	static bool			exists(const std::string &path);
	static bool			isDirectory(const std::string &path);
	static bool			isFile(const std::string &path);
	static bool			isReadable(const std::string &path);
	static bool			isWritable(const std::string &path);
	static size_t		getFileSize(const std::string &path);
	static std::string	getExtension(const std::string& path);
};

#endif