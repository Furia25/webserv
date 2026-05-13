/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestFactory.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 14:55:27 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/13 03:42:22 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _REQUESTFACTORY_H
# define _REQUESTFACTORY_H

# include <iostream>
# include <vector>
# include <string>
# include <stdint.h>
# include <sstream>
# include <cctype>
# include <exception>
# include <cstdlib>

# include "Utils/HashMap.hpp"
# include "HTTP/HttpTypes.hpp"
# include "HTTP/Request.hpp"

class RequestFactory
{
private:
	std::vector<uint8_t>	raw_buffer;
	bool					is_parsing_complete;
	bool					is_header_parsed;
	bool					is_validated;
	bool					is_chunked;

	std::string				method;
	std::string				request_path;
	std::string				query_string;
	std::string				protocol;

	Request::Headers		headers;

	size_t	findNewline(const std::vector<uint8_t>& buffer, size_t start, size_t max);
	size_t	findHeaderEnd();
	void	parseAllHeaders(const std::vector<uint8_t>& buffer, size_t pos);
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
	Request		build() const;

	void		print() const;

	std::vector<uint8_t>	getExtraData();

	const bool&		getCompleteStatus() 	const;
	const bool&		get_header_parsed() 	const;
	const bool&		isValidated() 			const;
	void			setValidateStatus(int status);
};

#endif // _REQUESTFACTORY_H