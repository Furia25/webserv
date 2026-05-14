/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antoine <antoine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 18:25:39 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/14 17:07:56 by antoine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _RESPONSE_H
# define _RESPONSE_H

# include "Server/Connection.hpp"
# include "HTTP/HttpTypes.hpp"
# include "Utils/HashMap.hpp"
# include "HTTP/HttpTypes.hpp"

class Response
{
private:
	HTTPCode							statusCode;
	std::string							body;
	HashMap<std::string, std::string>	headers;
	std::vector<std::string>			cookies;

	std::string	buildStatusLine() const;
    
public:
	Response(HTTPCode code = HTTPCode::OK);
	~Response();
    
	void	setStatusCode(HTTPCode code);
	void	setBody(const std::string &body);
	void	setHeader(const std::string &key, const std::string &value);
	void	setKeepAlive(bool keepAlive);
    void	setContentType(MIME mime_type);
    void	setContentLength(size_t length);
	void	addCookies(const std::string& cookie);

	std::string build() const;
	std::string buildHeadersOnly() const;
	
	std::string	sendChunk(const std::string& body);
	std::string	sendEndChunks();
};

#endif // _RESPONSE_H