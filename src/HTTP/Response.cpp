/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 15:59:45 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/15 06:00:34 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/Response.hpp"
# include "HTTP/HTTPTypes.hpp"
# include "Utils/IntegerUtils.hpp"

Response::Response(Connection& connection) : connection(connection), state(Response::STATUS), is_chunked(false) {}

Response::Response(Connection& connection, HTTPCode code) : connection(connection), state(Response::STATUS), is_chunked(false)
{
	this->sendStatusLine(code);
}

Response&	Response::sendHeader(const std::string& key, const std::string& value)
{
	if (state != Response::HEADER)
		throw std::runtime_error("Can't add headers");
	this->connection.sendData(key + ": " + value + HTTP_NEWLINE);
	return (*this);
}

Response& Response::sendStatusLine(HTTPCode code)
{
	if (this->state != Response::STATUS)
		throw std::runtime_error("Status already sent");
	this->connection.sendData(HTTP_VERSION " " + IntegerUtils::itoa(code) + " " + HTTPCode::toString(code) + HTTP_NEWLINE);
	this->state = Response::HEADER;
	return (*this);
}

Response& Response::setChunked()
{
	this->is_chunked = true;
	this->sendHeader("Transfer-Encoding", "chunked");
	return (*this);
}

Response& Response::sendDefaults(const Request& request, const Config::RouteConfig& route_config, bool force_close)
{
	this->sendHeader("Server", SERV_NAME "/" SERV_VERSION);
	if (force_close)
		this->sendKeepAlive(false);
	else
		this->sendKeepAlive(request.keep_alive);
	this->sendCookies(request.getCookies(), route_config.cookies);
	return (*this);
}

Response& Response::sendKeepAlive(bool keep_alive)
{
	if (keep_alive)
		this->sendHeader("Connection", "keep-alive");
	else
		this->sendHeader("Connection", "close");
	return (*this);
}

Response& Response::sendContentType(MIME mime_type)
{
	this->sendHeader("Content-Type", MIME::toString(mime_type));
	return (*this);
}

Response& Response::sendContentLength(size_t length)
{
	this->sendHeader("Content-Length", IntegerUtils::itoa(length));
	return (*this);
}

std::string	cookie_generate(size_t length)
{
	static const char charset[] =
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"0123456789";
	static const size_t charset_size = sizeof(charset) - 1;

	std::string result;
	result.reserve(length);

	int fd = open("/dev/urandom", O_RDONLY);
	if (fd == -1)
		throw std::runtime_error("cookie_generate: cannot open /dev/urandom");

	unsigned char buf[length];
	ssize_t n = read(fd, buf, length);
	close(fd);

	if (n == -1 || n != static_cast<ssize_t>(length))
		throw std::runtime_error("cookie_generate: read failed");

	for (size_t i = 0; i < length; ++i)
		result += charset[buf[i] % charset_size];

	return result;
}

Response& Response::sendCookies(const Cookies& cookies, const HashMap<std::string,
			Config::CookieConfig>& cookies_config)
{
	for (HashMap<std::string, Config::CookieConfig>::const_iterator it = cookies_config.begin();
			it != cookies_config.end(); ++it)
	{
		if (cookies.contain(it->first))
			continue ;
		const Config::CookieConfig&	config = it->second;
		if (config.required)
			continue ;
		std::string	value = config.default_value;
		if (config.generate)
			value += cookie_generate(config.generation_length);
		std::string	cookie_str = it->first + "=" + value;
		if (config.http_only)
			cookie_str += "; HttpOnly";
		switch (config.same_site)
		{
		case Config::CookieConfig::SameSite::LAX:
			cookie_str += "; SameSite=Lax";
			break;
		case Config::CookieConfig::SameSite::STRICT:
			cookie_str += "; SameSite=Strict";
			break;
		case Config::CookieConfig::SameSite::NONE:
			cookie_str += "; SameSite=None";
			break;
		}
		if (config.max_age != -1)
			cookie_str += "; Max-Age=" + IntegerUtils::itoa(config.max_age);
		this->sendHeader("Set-Cookie", cookie_str);
	}
	return (*this);
}

Response& Response::sendBody(const std::string& body)
{
	if (state == Response::END || state == Response::STATUS)
		throw std::runtime_error("Can't add body");
	if (state == Response::HEADER)
		this->connection.sendData(HTTP_NEWLINE);

	this->state = Response::BODY;
	this->connection.sendData(body);
	return (*this);
}

Response& Response::sendBody(const uint8_t *body, size_t length)
{
	if (state == Response::END || state == Response::STATUS)
		throw std::runtime_error("Can't add body");
	if (state == Response::HEADER)
		this->connection.sendData(HTTP_NEWLINE);

	this->state = Response::BODY;
	this->connection.sendData(body, length);
	return (*this);
}

Response& Response::sendChunk(const std::string& body)
{
	this->sendChunk(reinterpret_cast<const uint8_t *>(body.c_str()), body.size());
	return (*this);
}

Response& Response::sendChunk(const uint8_t *body, size_t length)
{
	if (!this->is_chunked || this->state == Response::END)
		throw std::runtime_error("Can't send chunk invalid state");
	if (state == Response::HEADER)
		this->connection.sendData(HTTP_NEWLINE);
	this->state = Response::BODY;
	std::stringstream ss;
	ss << std::hex << length;

	this->connection.sendData(ss.str() + HTTP_NEWLINE);
	this->connection.sendData(body, length);
	this->connection.sendData(HTTP_NEWLINE);
	return (*this);
}

void	Response::sendEnd()
{
	if (this->state == Response::END || this->state == Response::STATUS)
		return ;
	if (this->state == Response::HEADER)
		this->connection.sendData(HTTP_NEWLINE);
	if (this->is_chunked == true)
		this->connection.sendData("0" HTTP_NEWLINE HTTP_NEWLINE);
	this->state = Response::END;
}
