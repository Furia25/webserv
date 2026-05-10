/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 15:24:08 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/11 01:46:16 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _REQUEST_H
# define _REQUEST_H

# include <string>
# include <vector>
# include "EnumClass.hpp"
# include "Utils/HashMap.hpp"
# include "HttpTypes.hpp"
# include "Utils/FileWriter.hpp"

class Request
{
public:
	typedef HashMap<std::string, std::string> Cookies;
	typedef HashMap<std::string, std::string> Headers;

	Method								method;
	std::string							path;
	std::string							query_string;
	std::string							protocol;
	size_t								content_length;

	Request() : content_length(0) {};

	void	appendToBody(const uint8_t* data, size_t size);
	void	reserveBody(size_t size);

	const std::vector<uint8_t>&					getBody()			const;
	size_t										getBodySize()		const;
	size_t										isLessThanOneMO()	const;
	const Headers&	getHeaders() 		const;

	void	setCookies(const Cookies& cookies);
	void	setHeaders(const Headers& headers);
private:
	Headers					headers;
	Cookies					cookies;
	std::vector<uint8_t>	body;
};

#endif
