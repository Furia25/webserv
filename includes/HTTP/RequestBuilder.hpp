/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestBuilder.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antoine <antoine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 14:55:27 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/24 12:51:48 by antoine          ###   ########.fr       */
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
	uint8_t					raw_buffer[MAX_HEADER_SIZE];
	size_t					buffer_size;
	bool					is_parsing_complete;
	bool					is_header_parsed;
	bool					is_validated;

	std::string				method;
	std::string				request_path;
	std::string				query_string;
	std::string				protocol;

	Headers					headers;

	size_t		findNewline(const uint8_t* buffer, size_t start, size_t max);
	size_t		findHeaderEnd();
	void		parseAllHeaders(const uint8_t* buffer, size_t pos);
	void		parseRequestLine(std::string &line);
	void		parseHeaderLine(std::string &line);
	void		toLowerCase(std::string &str);
	void		validateMethod() const;
	void		validateProtocol() const;
	void		validatePath();
	void		validateHeader() const;
	void		invalidPath();

	const std::string *getHeader(const std::string& key) const;

public:

	RequestBuilder();
	void		feed(const uint8_t *fragment, size_t length);
	void		check();
	void		reset();
	Request		build() const;

	void		print() const;

	std::vector<uint8_t>	getExtraData();

	const bool&		getCompleteStatus() 	const;
	const bool&		get_header_parsed() 	const;
	const bool&		isValidated() 			const;
	void			setValidateStatus(int status);
};


#endif // _RequestBuilder_H