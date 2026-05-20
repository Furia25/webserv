/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIFactory.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/20 18:42:09 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/20 19:03:59 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _CGIFACTORY_H
# define _CGIFACTORY_H

# include <iostream>
# include <vector>
# include "HTTP/RequestFactory.hpp"

# define MAX_CGI_BUFFER 8192

class CGIResponseParser
{
public:
	void feed(const uint8_t* data, size_t len);
	bool isHeaderParsed() const;
	size_t	tryParseHeaderLine(const uint8_t* buffer, size_t size, std::string &key, std::string& value);

private:
	uint8_t				buffer[MAX_CGI_BUFFER];
	size_t				buffer_size;
	bool				header_done;
	bool				first_line_done;
	Headers				headers;
	int					status_code;
};

#endif // _CGIFACTORY_H