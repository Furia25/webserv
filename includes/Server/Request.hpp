/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 19:17:15 by antoine           #+#    #+#             */
/*   Updated: 2026/03/25 16:05:03 by antbonin         ###   ########.fr       */
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
	const bool &getCompleteStatus() const;
	const std::string &getMethod() const;
	const std::string &getRequestPath() const;
	const std::string &getProtocol() const;

  private:
	HashMap<std::string, std::string> headers;
	std::string method;
	std::string request_path;
	std::string protocol;
	std::vector<uint8_t> body;
	
	std::vector<uint8_t> raw_buffer;
	bool is_complete;
	bool header_parsed;
	size_t content_length;
	
	void toLowerCase(std::string &str);
	void parseRequestLine(std::string line);
	void parseHeaderLine(std::string &line);
	void validateMethod() const;
	void validateProtocol() const;
	void validatePath();
	void validateHeader();
	int checkPathType(const std::string &path);
	std::string trim(const std::string &str) const;
};