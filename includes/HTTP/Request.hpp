/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 15:24:08 by antbonin          #+#    #+#             */
/*   Updated: 2026/06/11 16:55:55 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _REQUEST_H
# define _REQUEST_H

# include <string>
# include <vector>
# include "EnumClass.hpp"
# include "Utils/HashMap.hpp"
# include "HTTPTypes.hpp"
# include "HTTP/Utils/FileWriter.hpp"

class Request
{
public:
	std::string		path;
	Method			method;
	std::string		protocol;
	bool			is_chunked;
	bool			keep_alive;
	size_t			content_length;
	std::string		query_string;

	Request();

	const std::string	*operator[](const std::string& key) const;
	const std::string	*operator[](const char *key) const;

	const Headers&		getHeaders()		const;
	const Cookies&		getCookies()		const;
	Cookies&			getCookies();
	Headers&			getHeaders();

	void				setCookies(const Cookies& cookies);
	void				setHeaders(const Headers& headers);
	void				reset();

private:
	Headers			headers;
	Cookies			cookies;
};

#endif
