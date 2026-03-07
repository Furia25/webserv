/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antoine <antoine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 19:17:15 by antoine           #+#    #+#             */
/*   Updated: 2026/03/07 16:09:57 by antoine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <stdint.h>
#include <vector>

static const std::string allowed_method[] = {"GET", "POST", "DELETE"};

enum	Method
{
	GET,
	POST,
	METHOD,
	UNKNOWN,
};

class Request
{
  public:
	Request();
	Request(const Request &other);
	~Request();
	Request &operator=(const Request &other);
	void feed(const std::vector<uint8_t> &fragment);

	// Getters
	const bool &is_complete() const;
	const std::string &getMethod() const;
	const std::string &getRequestPath() const;
	const std::string &getProtocol() const;

  private:
	std::string method;
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