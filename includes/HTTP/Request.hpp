/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 15:24:08 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/08 17:17:44 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _REQUEST_H
# define _REQUEST_H

# include <string>
# include <vector>
# include "EnumClass.hpp"
# include "Utils/HashMap.hpp"
# include "HttpTypes.hpp"
# include "Utils/FileWriter.hpp"

class Body
{
private:
	FileWriter*				fileWriter;
	bool					isStreaming;
	size_t					expectedSize;
	size_t					receivedSize;
	std::vector<uint8_t>	memoryBuffer;
	std::string				destinationPath;

public:
	Body();
	
	~Body();
	
	void			feed(const uint8_t* data, size_t size);
	void			init(size_t expected, const std::string& path, bool stream);

	void			finish();
	void			reset();

	void			setIsStreaming(bool stream);
	void			setFilePath(const std::string& path);

	bool							isComplete()			const;
	const std::string&				getFilePath()			const;
	FileWriter*						getFileWriter()			const;
	bool							checkOverflow()			const;
	bool							getIsStreaming()		const;
	size_t							getReceivedSize()		const;
	const std::vector<uint8_t>&		getMemoryBuffer()		const;
	
};

class Request
{
private:
	Method								method;
	std::string							path;
	std::string							query_string;
	std::string							protocol;
	size_t								content_length;
	HashMap<std::string, std::string>	headers;
	Body								body;

public:
	Request() {};
	Request(Method m, const std::string& p, const std::string& q, 
			const std::string& proto, size_t cl, 
			const HashMap<std::string, std::string>& h);
	~Request();

		
	void		initBody(const std::string& path, bool stream);
	void		feedBody(const uint8_t* data, size_t size);
	void		finishBody();
	
	bool		isBodyComplete()	const;
	bool		checkBodyOverflow() const;
	
	Body&										getBody();
	const Body&									getBody()			const;
	size_t										getBodySize()		const;
	size_t										isLessThanOneMO()	const;
	Method										getMethod() 		const;
	const std::string&							getPath() 			const;
	const std::string&							getQueryString() 	const;
	const std::string&							getProtocol() 		const;
	size_t										getContentLength() 	const;
	const HashMap<std::string, std::string>&	getHeaders() 		const;
};

#endif
