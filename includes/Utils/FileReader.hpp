/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileReader.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 11:36:35 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/28 14:28:04 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _FILEREADER_H
# define _FILEREADER_H

# include <iostream>
# include <fstream>
# include <vector>
# include <stdint.h>
class FileReader
{
public:
	FileReader();
	~FileReader();
	
	void	open(const std::string &path);
	size_t	readChunk(std::vector<uint8_t>& buffer, size_t chunkSize = 8192);

	size_t		getFileSize() const;
	bool		hasFinished() const;
	void		close();

private:
	std::ifstream	fileStream;
	std::string		filePath;
	size_t 			fileSize;
	size_t			bytesReadTotal;
	bool			isEOF;
};

#endif // _FILEREADER_H