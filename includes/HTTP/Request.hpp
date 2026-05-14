/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antoine <antoine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 15:24:08 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/14 16:53:46 by antoine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _REQUEST_H
# define _REQUEST_H

# include <string>
# include <vector>
# include "EnumClass.hpp"
# include "Utils/HashMap.hpp"
# include "HttpTypes.hpp"
# include "HTTP/Utils/FileWriter.hpp"

class Request
{
public:
	Method			method;
	std::string		path;
	std::string		query_string;
	std::string		protocol;
	size_t			content_length;
	bool			is_chunked;
	bool			keep_alive;

	Request();

	typedef HashMap<std::string, std::string> Cookies;
	typedef HashMap<std::string, std::string> Headers;

	const Headers&		getHeaders()		const;
	const Cookies&		getCookies()		const;
	Cookies&			getCookies();
	Headers&			getHeaders();
	bool				wantsKeepAlive()	const;

	void				setCookies(const Cookies& cookies);
	void				setHeaders(const Headers& headers);

private:
	Headers			headers;
	Cookies			cookies;
};

#endif
