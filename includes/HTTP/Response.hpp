/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 18:25:39 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/22 05:42:07 by vdurand          ###   ########.fr       */
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
public:
	Response(Connection& connection);
	Response(Connection& connection, HTTPCode code);

	Response&	sendStatusLine(HTTPCode code);
	Response&	sendStatusLine(size_t code);
	Response&	sendStatusLine(const std::string& code_full_str);
	Response&	setChunked();
	Response&	sendDefaults(const Request& request,
					const Config::RouteConfig *route_config, bool force_close = false);
	Response&	sendKeepAlive(bool keep_alive);
	Response&	sendContentType(MIME mime_type);
	Response&	sendContentLength(size_t length);
	Response&	sendHeaders(const Headers& headers);
	Response&	sendCookies(const Cookies& cookies,
					const HashMap<std::string, Config::CookieConfig>& cookies_config);
	Response&	sendCookie(const std::string& key, const std::string& value,
					bool http_only = false, Cookie::SameSite same_site = Cookie::SameSite::LAX, int64_t max_age = -1);

	void		sendEnd();

	Response&	sendBody(const std::string& body);
	Response&	sendBody(const uint8_t *body, size_t length);
	Response&	sendChunk(const std::string& body);
	Response&	sendChunk(const uint8_t *body, size_t length);

	bool		hasStatus() const { return this->state == Response::STATUS; };

	void		setBuffering(bool enable) { this->isBuffered = enable; };

	inline Response&	sendHeader(const std::string& key, const std::string& value)
	{
		if (this->state != Response::HEADER)
			throw std::runtime_error("Can't add headers");
		this->sendData(key);
		this->sendData(": ");
		this->sendData(value);
		this->sendData(HTTP_NEWLINE);
		return (*this);
	};

private:
	enum State	{STATUS, HEADER, BODY, END};

	Connection&				connection;
	enum State				state;
	bool					isChunked;

	bool					isBuffered;
	std::vector<uint8_t>	buffer;

	inline void	sendData(const uint8_t *data, size_t len)
	{
		if (!this->isBuffered)
		{
			if (!this->buffer.empty())
			{
				this->connection.sendData(this->buffer.data(), this->buffer.size());
				this->buffer.clear();
			}
			this->connection.sendData(data, len);
		}
		else
			this->buffer.insert(this->buffer.end(), data, data + len);
	};

	inline void	sendData(const std::string &data)
	{
		this->sendData(reinterpret_cast<const uint8_t *>(data.c_str()), data.size());
	};

	Response(const Response& other);
	Response&	operator=(const Response& other);
};

#endif // _RESPONSE_H