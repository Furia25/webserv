/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antoine <antoine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 15:24:08 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/13 00:30:09 by antoine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _REQUEST_H
# define _REQUEST_H

# include <string>
# include <vector>
# include "EnumClass.hpp"
# include "Utils/HashMap.hpp"
# include "HttpTypes.hpp"
# include "Utils/HTTP/FileWriter.hpp"

class Request
{
	public:
	Method			method;
	std::string		path;
	std::string		query_string;
	std::string		protocol;
	size_t			content_length;
	bool			is_chunk_encoding;

	typedef HashMap<std::string, std::string> Cookies;
	typedef HashMap<std::string, std::string> Headers;

	Request();
	Request(const Request& other);
	Request& operator=(const Request& other);
	~Request();

	Method				getMethod()			const;
	const std::string&	getPath()			const;
	const std::string&	getQueryString()	const;
	const std::string&	getProtocol()		const;
	size_t				getContentLength()	const;
	bool				isChunked()			const;

	const Headers&		getHeaders()		const;
	const Cookies&		getCookies()		const;
	Cookies&			getCookies();
	Headers&			getHeaders();

	void				setCookies(const Cookies& cookies);
	void				setHeaders(const Headers& headers);

private:

	Headers			headers;
	Cookies			cookies;
};

#endif
