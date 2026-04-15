/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 19:17:15 by antoine           #+#    #+#             */
/*   Updated: 2026/04/16 00:25:06 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include <iostream>
# include <map>
# include <sstream>
# include <stdexcept>
# include <stdint.h>
# include <vector>

# include "EnumClass.hpp"
# include "Utils/HashMap.hpp"
# include "HttpTypes.hpp"

class Request
{
	
public:
	Request();
	Request(const Request &other);
	~Request();
	Request 	&operator=(const Request &other);
	void 		feed(const uint8_t *buffer, size_t length);
	void		check();
	void		print() const;

	const bool 			&getCompleteStatus() const;
	const std::string 	&getMethod() const;
	const std::string 	&getRequestPath() const;
	const std::string 	&getProtocol() const;
	const bool			&isHeaderParsed()const ;
	const bool			&isValidated()const ;
	void				setValidateStatus(int status);

private:
	std::vector<uint8_t> 			raw_buffer;
	bool 							parsing_is_complete;
	bool 							header_is_parsed;
	bool							is_validated;
	size_t							content_length;
  
	HashMap<std::string, std::string> 	headers;
	std::string 						method;
  	std::string 						request_path;
  	std::string 						query_path;
	std::string 						protocol;
	std::vector<uint8_t> 				body;

	void 			toLowerCase(std::string &str);
	size_t 			find_header_end();
	void			parse_all_headers(const std::vector<uint8_t>& buffer, size_t pos);
	size_t 			find_newline(const std::vector<uint8_t>& buffer, size_t start, size_t max);
	void 			parseRequestLine(std::string &line);
	void 			parseHeaderLine(std::string &line);
	void 			validateMethod() const;
	void 			validateProtocol() const;
	void 			validateHeader();
	void 			validatePath();
	void 			invalidPath();
};
