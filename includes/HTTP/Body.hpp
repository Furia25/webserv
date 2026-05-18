/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Body.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/12 17:07:09 by antoine           #+#    #+#             */
/*   Updated: 2026/05/13 02:07:32 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BODY_HPP
# define BODY_HPP

# include "HTTP/Utils/FileWriter.hpp"

class Body
{
private:
	FileWriter*				fileWriter;
	bool					isStreaming;
	size_t					expectedSize;
	size_t					receivedSize;
	std::vector<uint8_t>	memoryBuffer;
	std::string				destinationPath;
	bool					isFinished;

public:
	Body();
	Body(const Body& other);
	Body& operator=(const Body& other);
	~Body();
	
	void			feed(const uint8_t* data, size_t size);
	void			init(size_t expected, const std::string& path, bool stream);

	void			finish();
	void			reset();
	
	void 			setIsFinished(bool status);
	void			setIsStreaming(bool stream);
	void			setFilePath(const std::string& path);
	
	bool							isOpen()				const;
	size_t							getSize()				const;
	bool							isComplete()			const;
	const std::string&				getFilePath()			const;
	FileWriter*						getFileWriter()			const;
	bool							checkOverflow()			const;
	bool							getIsStreaming()		const;
	bool							isLessThanOneMO()		const;
	size_t							getReceivedSize()		const;
	const std::vector<uint8_t>&		getMemoryBuffer()		const;
	
};

# endif