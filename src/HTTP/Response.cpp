/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 15:59:45 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/15 04:18:18 by vdurand          ###   ########.fr       */
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

Response& Response::sendDefaults(const Request& request, const Config::RouteConfig& route_config)
{
	this->sendHeader("Server", SERV_NAME "/" SERV_VERSION);
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

Response& Response::sendCookies(const Cookies& cookies, const HashMap<std::string,
			Config::CookieConfig>& cookies_config)
{
	(void) cookies;
	(void) cookies_config;
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
