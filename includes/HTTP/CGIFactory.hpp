/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIFactory.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/20 18:42:09 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/21 00:58:45 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _CGIFACTORY_H
# define _CGIFACTORY_H

# include <iostream>
# include <vector>
# include "HTTP/RequestFactory.hpp"

# define MAX_CGI_BUFFER 4096

class CGIResponseParser
{
public:
	void	feed(const uint8_t* data, size_t len);
	bool	isHeaderParsed() const;
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