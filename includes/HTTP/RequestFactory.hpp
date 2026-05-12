/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestFactory.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 14:55:27 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/12 16:37:11 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _RequestFactory_H
# define _RequestFactory_H

# include <iostream>
# include <vector>
# include <string>
# include <stdint.h>
# include <sstream>
# include <cctype>
# include <exception>
# include <cstdlib>

# include "Utils/HashMap.hpp"
# include "HttpTypes.hpp"
# include "Request.hpp"

#define _ENCODING_CHUNK_ "transfer-encoding"

class RequestFactory
{
private:
	std::vector<uint8_t>	raw_buffer;
	bool					is_parsing_complete;
	bool					is_header_parsed;
	bool					is_validated;
	bool					is_chunk_encoding;

	std::string				method;
	std::string				request_path;
	std::string				query_string;
	std::string				protocol;

	Request::Headers		headers;

	size_t	find_newline(const std::vector<uint8_t>& buffer, size_t start, size_t max);
	size_t	find_header_end();
	void	parse_all_headers(const std::vector<uint8_t>& buffer, size_t pos);
	void	parseRequestLine(std::string &line);
	void	parseHeaderLine(std::string &line);
	void	toLowerCase(std::string &str);
	void	validateMethod() const;
	void	validateProtocol() const;
	void	validatePath();
	void	validateHeader();
	void	invalidPath();

	const std::string *getHeader(const std::string& key) const;

public:
	RequestFactory();
	RequestFactory(const RequestFactory &other);
	RequestFactory &operator=(const RequestFactory &other);
	~RequestFactory();

	void		feed(const uint8_t *fragment, size_t length);
	void		check();
	void		reset();
	Request*	build() const;

	void		print() const;

	std::vector<uint8_t>	getExtraData();

	const bool&		getCompleteStatus() 	const;
	const bool&		get_header_parsed() 	const;
	const bool&		isValidated() 			const;
	void			setValidateStatus(int status);
};

#endif // _RequestFactory_H