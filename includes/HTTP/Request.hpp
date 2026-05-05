/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 15:24:08 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/05 12:46:53 by antbonin         ###   ########.fr       */
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
private:
	Method								method;
	std::string							path;
	std::string							query_string;
	std::string							protocol;
	size_t								content_length;
	HashMap<std::string, std::string>	headers;
	std::vector<uint8_t>				body;

public:
	Request(Method m, const std::string& p, const std::string& q, 
			const std::string& proto, size_t cl, 
			const HashMap<std::string, std::string>& h);
	~Request();

	void	appendToBody(const uint8_t* data, size_t size);
	void	reserveBody(size_t size);
	
	const std::vector<uint8_t>&					getBody()			const;
	size_t										getBodySize()		const;
	size_t										isLessThanOneMO()	const;
	Method										getMethod() 		const;
	const std::string&							getPath() 			const;
	const std::string&							getQueryString() 	const;
	const std::string&							getProtocol() 		const;
	size_t										getContentLength() 	const;
	const HashMap<std::string, std::string>&	getHeaders() 		const;
};

#endif
