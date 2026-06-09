/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Body.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/12 17:07:09 by antoine           #+#    #+#             */
/*   Updated: 2026/06/09 19:56:07 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BODY_HPP
# define BODY_HPP

# include "HTTP/Utils/FileWriter.hpp"

class Body
{
private:
	enum ChunkState
	{
		CHUNK_SIZE,
		CHUNK_DATA,
		CHUNK_TRAILER,
		CHUNK_COMPLETE
	};

	FileWriter				*fileWriter;
	size_t					expectedSize;
	size_t					receivedSize;
	std::string				destinationPath;
	bool					isFinished;

	ChunkState				chunkState;
	size_t					neededBytes;
	std::string				sizeBuffer;
	size_t					maxBodySize;

public:
	Body();
	Body(const Body& other);
	Body& operator=(const Body& other);
	~Body();
	
	void			feed(const uint8_t* data, size_t size);
	void			init(size_t expected, const std::string& path, size_t maxBodySize);

	void			finish();
	void			reset();
	
	void 			setIsFinished(bool status);
	void			setFilePath(const std::string& path);
	
	void			feedChunked(const uint8_t* fragment, size_t size);
	void			handleChunkSize(const uint8_t* fragment, size_t& i);
	void			handleChunkData(const uint8_t* fragment, size_t& i, size_t size);
	void			handleChunkTrailer(const uint8_t* fragment, size_t& i);
	
	size_t							getSize()				const;
	bool							isComplete()			const;
	bool							hasFinished()			const;
	const std::string&				getFilePath()			const;
	FileWriter*						getFileWriter()			const;
	bool							checkOverflow()			const;
	size_t							getReceivedSize()		const;
	
};

# endif