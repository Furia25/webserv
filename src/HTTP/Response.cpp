/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 15:59:45 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/05 14:38:06 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/Response.hpp"
# include "HTTP/HttpTypes.hpp"
# include "Utils/Itoa.hpp"

static inline std::string buildStatusLine(HTTPCode code)
{
	return "HTTP/1.1 " + itoa(code) + " " + HTTPCode::toString(code) + "\r\n";
}

void Response::buildErrorResponse(Connection& connection, HTTPCode code)
{
	std::stringstream ss;
	ss << "<html><head><title>Error " << static_cast<int>(code) << "</title></head>" 
		<< "<body><center><h1>" << static_cast<int>(code) << "</h1><hr>"
		<< "WebServ/1.0 (Defautl Error Page)</center></body></html>";

	std::string body = ss.str();
	Response::buildRawResponse(connection, code, MIME::html, body);
}

void Response::buildRawResponse(Connection& connection, HTTPCode code, MIME mime_type, const std::string& body)
{
	std::string response = buildStatusLine(code);
	response += "Content-Type: " + std::string(MIME::toString(mime_type)) + "\r\n";
	response += "Content-Length: " + itoa(body.size()) + "\r\n";
	response += "Connection: close\r\n\r\n";
	response += body;
	
	connection.sendData(response);
}

void Response::buildEmptyResponse(Connection& connection, HTTPCode code)
{
	std::string response = buildStatusLine(code);
	response += "Connection: close\r\n\r\n";
	
	connection.sendData(response);
}

void Response::buildFileHeaderResponse(Connection& connection, HTTPCode code, MIME mime_type, size_t fileSize)
{
	std::string response = buildStatusLine(code);
	response += "Content-Type: " + std::string(MIME::toString(mime_type)) + "\r\n";
	response += "Content-Length: " + itoa(fileSize) + "\r\n";
	response += "Connection: close\r\n\r\n";
	
	connection.sendData(response);
}

void Response::sendBodyChunk(Connection& connection, const uint8_t* data, size_t len)
{
	connection.sendData(data, len);
}
