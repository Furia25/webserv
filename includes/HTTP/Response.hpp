/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 18:25:39 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/15 04:18:32 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _RESPONSE_H
# define _RESPONSE_H

# include "HTTP/Body.hpp"
# include "Server/Connection.hpp"
# include "HTTP/HTTPTypes.hpp"
# include "Utils/HashMap.hpp"
# include "Config/Config.hpp"
# include "Request.hpp"

class Response
{
private:
	enum State	{STATUS, HEADER, BODY, END};

	Connection&	connection;
	enum State	state;
	bool		is_chunked;

public:
	Response(Connection& connection);
	Response(Connection& connection, HTTPCode code);

	Response&	sendStatusLine(HTTPCode code);
	Response&	setChunked();
	Response&	sendDefaults(const Request& request, const Config::RouteConfig& route_config);
	Response&	sendKeepAlive(bool keep_alive);
	Response&	sendContentType(MIME mime_type);
	Response&	sendContentLength(size_t length);
	Response&	sendHeader(const std::string& key, const std::string& value);
	Response&	sendCookies(const Cookies& cookies,
					const HashMap<std::string, Config::CookieConfig>& cookies_config);

	void		sendEnd();

	Response&	sendBody(const std::string& body);
	Response&	sendBody(const uint8_t *body, size_t length);
	Response&	sendChunk(const std::string& body);
	Response&	sendChunk(const uint8_t *body, size_t length);
};

#endif // _RESPONSE_H