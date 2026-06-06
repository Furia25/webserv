/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Body.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/12 17:07:09 by antoine           #+#    #+#             */
/*   Updated: 2026/06/06 17:43:16 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BODY_HPP
# define BODY_HPP

# include "HTTP/Utils/FileWriter.hpp"

class Body
{
private:
	FileWriter				*fileWriter;
	bool					isStreaming;
	size_t					expectedSize;
	size_t					receivedSize;
	std::vector<uint8_t>	memoryBuffer;
	std::string				destinationPath;
	bool					isFinished;
	

	enum ChunkState
	{
		CHUNK_SIZE,
		CHUNK_DATA,
		CHUNK_TRAILER,
		CHUNK_COMPLETE
	};
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
	void			init(size_t expected, const std::string& path, bool stream, size_t maxBodySize);

	void			finish();
	void			reset();
	
	void 			setIsFinished(bool status);
	void			setIsStreaming(bool stream);
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
	bool							getIsStreaming()		const;
	bool							isLessThanOneMO()		const;
	size_t							getReceivedSize()		const;
	const std::vector<uint8_t>&		getMemoryBuffer()		const;
	
};

# endif