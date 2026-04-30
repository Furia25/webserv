/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FileWriter.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/29 13:49:41 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/30 14:20:49 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _FILEWRITER_H
# define _FILEWRITER_H

# include <string>
# include <vector>
# include <fstream>
# include <stdint.h>

class FileWriter
{
public:
	FileWriter(): bytesWrittenTotal(0), hasError(false) {};
	~FileWriter(){this->close();};
	
	void			open(const std::string& path, bool append = false);
	size_t			writeChunk(const void* data, size_t size);
	size_t			writeChunk(const char* data, size_t size);
	void			close();
	void			abort();

	size_t				getBytesWritten()	const ;
	bool				getError()			const ;
	const std::string&	getFilePath()		const ;

private:
	std::string 		filePath;
	std::ofstream 		fileStream;
	size_t				bytesWrittenTotal;
	bool				hasError;
};

#endif // _FILEWRITER_H