/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 19:17:15 by antoine           #+#    #+#             */
/*   Updated: 2026/03/26 16:22:14 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include <iostream>
# include <map>
# include <sstream>
# include <stdexcept>
# include <stdint.h>
# include <vector>
# include "EnumClass.hpp"
# include "HashMap.hpp"

# define _METHOD_TYPE \
	(GET, POST, DELETE)

ENUM_CLASS(MethodType, _METHOD_TYPE, ENUM_BASIC, ENUM_LITERALS(_METHOD_TYPE, ENUM_BASIC, ENUM_BASIC_STRING););


class Request
{
	
public:
	Request();
	Request(const Request &other);
	~Request();
	Request &operator=(const Request &other);
	void feed(const uint8_t *buffer, size_t length);

	// Getters
	const bool 			&getCompleteStatus() const;
	const std::string 	&getMethod() const;
	const std::string 	&getRequestPath() const;
	const std::string 	&getProtocol() const;

private:

  
	std::vector<uint8_t> 			raw_buffer;
	bool 							is_complete;
	bool 							header_parsed;
	size_t							content_length;
  
	HashMap<std::string, std::string> 	headers;
	std::string 						method;
  	std::string 						request_path;
  	std::string 						query_path;
	std::string 						protocol;
	std::vector<uint8_t> 				body;
	
	
	void 			toLowerCase(std::string &str);
	size_t 			Request::find_header_end();
	void			Request::parse_all_headers(const std::vector<uint8_t>& buffer, size_t pos);
	size_t 			Request::find_newline(const std::vector<uint8_t>& buffer, size_t start, size_t max);
	void 			parseRequestLine(std::string &line);
	void 			parseHeaderLine(std::string &line);
	void 			validateMethod() const;
	void 			validateProtocol() const;
	void 			validateHeader();
	void 			validatePath();
	void 			invalidPath();
	int 			checkPathType(const std::string &path);
	std::string 	trim(const std::string &str) const;
};