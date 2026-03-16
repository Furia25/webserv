/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 19:17:15 by antoine           #+#    #+#             */
/*   Updated: 2026/03/16 19:37:05 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include <iostream>
# include <map>
# include <sstream>
# include <stdexcept>
# include <stdint.h>
# include <vector>
# include "EnumClass.hpp"

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
	void feed(uint8_t *buffer);

	// Getters
	const bool &getCompleteStatus() const;
	const std::string &getMethod() const;
	const std::string &getRequestPath() const;
	const std::string &getProtocol() const;

  private:
	MethodType method;
	std::string request_path;
	std::string protocol;
	std::map<std::string, std::string> headers;
	std::vector<uint8_t> body;
	
	std::vector<uint8_t> raw_buffer;
	bool is_complete;
	bool header_parsed;
	size_t content_length;
	
	void parseRequestLine(std::string line);
	void parseHeaderLine(std::string line);
	void validateMethod() const;
	void validateProtocol() const;
	void validatePath() const;
	std::string trim(const std::string &str) const;
};