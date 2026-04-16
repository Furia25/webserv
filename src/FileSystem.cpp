/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileSystem.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 16:55:00 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/16 17:01:17 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Utils/FileSystem.hpp"

static bool exists(const std::string& path) 
{
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

static bool isDirectory(const std::string& path) 
{
    struct stat buffer;
    if (stat(path.c_str(), &buffer) != 0) return false;
        return S_ISDIR(buffer.st_mode);
}

static bool isFile(const std::string& path) 
{
    struct stat buffer;
    if (stat(path.c_str(), &buffer) != 0) return false;
        return S_ISREG(buffer.st_mode);
}

static bool isReadable(const std::string& path) 
{
    return (access(path.c_str(), R_OK) == 0);
}

static bool isWritable(const std::string& path) 
{
    return (access(path.c_str(), W_OK) == 0);
}