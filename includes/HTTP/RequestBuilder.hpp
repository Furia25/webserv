/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestBuilder.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antoine <antoine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 14:55:27 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/09 23:18:31 by antoine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _REQUESTBUILDER_H
# define _REQUESTBUILDER_H

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

class RequestBuilder
{
private:
	std::vector<uint8_t>				raw_buffer;
	bool								parsing_is_complete;
	bool								header_is_parsed;
	bool								is_validated;
	bool								is_chunk_encoding;

	size_t								content_length;
	HashMap<std::string, std::string>	headers;
	std::string							method;
	std::string							request_path;
	std::string							query_path;
	std::string							protocol;
	

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

public:
	RequestBuilder();
	RequestBuilder(const RequestBuilder &other);
	RequestBuilder &operator=(const RequestBuilder &other);
	~RequestBuilder();

	void		feed(const uint8_t *fragment, size_t length);
	void		check();
	void		reset();
	Request		build() const;

	void		print() const;

	std::vector<uint8_t>	getExtraData();

	const bool&		getCompleteStatus() 	const;
	const bool&		isHeaderParsed() 		const;
	const bool&		isValidated() 			const;
	void			setValidateStatus(int status);
};

#endif // _REQUESTBUILDER_H