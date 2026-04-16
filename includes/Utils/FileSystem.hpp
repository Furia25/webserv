/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileSystem.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 16:51:11 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/16 17:01:09 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


# ifndef _FILESYSTEM_H
# define _FILESYSTEM_H

# include <iostream>
# include <sys/stat.h>
# include <unistd.h>

class FileSystem
{
public:

    static bool exists(const std::string &path);
    static bool isDirectory(const std::string &path);
    static bool isFile(const std::string &path);
    static bool isReadable(const std::string &path);
    static bool isWritable(const std::string &path);
private:
    FileSystem();
    FileSystem(const FileSystem& other);
    ~FileSystem();
    FileSystem& operator=(const FileSystem& other);
};

#endif