/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileInsert.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 13:49:41 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/29 13:51:44 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _FILEINSERT_H
# define _FILEINSERT_H

class FileInsert
{
public:
	FileInsert();
	~FileInsert();
	
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

#endif // _FILEINSERT_H