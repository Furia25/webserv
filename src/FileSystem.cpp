/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileSystem.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 16:55:00 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/21 16:01:06 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Utils/FileSystem.hpp"

bool FileSystem::exists(const std::string& path) 
{
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

bool FileSystem::isDirectory(const std::string& path) 
{
    struct stat buffer;
    if (stat(path.c_str(), &buffer) != 0) return false;
        return S_ISDIR(buffer.st_mode);
}

bool FileSystem::isFile(const std::string& path) 
{
    struct stat buffer;
    if (stat(path.c_str(), &buffer) != 0) return false;
        return S_ISREG(buffer.st_mode);
}

bool FileSystem::isReadable(const std::string& path) 
{
    return (access(path.c_str(), R_OK) == 0);
}

bool FileSystem::isWritable(const std::string& path) 
{
    return (access(path.c_str(), W_OK) == 0);
}

size_t FileSystem::getFileSize(const std::string &path)
{
    struct stat buffer;
    if (stat(path.c_str(), &buffer) == 0)
        return buffer.st_size;
    return 0;
}

std::string FileSystem::getExtension(const std::string& path)
{
    size_t pos = path.find_last_of('.');
    if (pos != std::string::npos)
        return path.substr(pos + 1);
    return "";
}
