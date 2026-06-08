/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestBuilder.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 14:55:27 by antbonin          #+#    #+#             */
/*   Updated: 2026/06/08 14:43:46 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _RequestBuilder_H
# define _RequestBuilder_H

# include <iostream>
# include <vector>
# include <string>
# include <stdint.h>
# include <sstream>
# include <cctype>
# include <exception>
# include <cstdlib>
# include <cstring>

# include "Utils/HashMap.hpp"
# include "HTTP/HTTPTypes.hpp"
# include "HTTP/Request.hpp"

# define MAX_HEADER_SIZE 4096

class RequestBuilder
{
private:
	size_t					buffer_size;
	bool					is_validated;
	bool					is_header_parsed;
	bool					is_parsing_complete;
	uint8_t					raw_buffer[MAX_HEADER_SIZE];

	std::string				method;
	std::string				protocol;
	std::string				request_path;
	std::string				query_string;
	
	Headers					headers;

	size_t		findHeaderEnd();
	void		toLowerCase(std::string &str);
	void		parseHeaderLine(std::string &line);
	void		parseRequestLine(std::string &line);
	void		parseAllHeaders(const uint8_t* buffer, size_t pos);
	size_t		findNewline(const uint8_t* buffer, size_t start, size_t max);
	
	void		invalidPath();
	void		validatePath();
	void		validateMethod() 	const;
	void		validateHeader() 	const;
	void		validateProtocol() 	const;

	const std::string *getHeader(const std::string& key) const;

public:

	RequestBuilder();
	
	void		check();
	void		reset();
	void		setValidateStatus(int status);
	void		feed(const uint8_t *fragment, size_t length);
	
	Request		build() const;
	
	
	void					print() 				const;
	const bool&				isValidated() 			const;
	const bool&				getCompleteStatus() 	const;
	const bool&				get_header_parsed() 	const;
	std::vector<uint8_t>	getExtraData();
};


#endif // _RequestBuilder_H