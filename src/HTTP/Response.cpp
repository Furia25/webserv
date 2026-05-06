/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 15:59:45 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/06 18:00:21 by vdurand          ###   ########.fr       */
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
		<< "<body><center><h1>" << static_cast<int>(code) << " : " << HTTPCode::toString(code) << "</h1><hr>"
		<< SERV_NAME "/" SERV_VERSION " (Default Error Page)</center></body></html>";

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

void Response::sendChunkedHeader(Connection& connection, HTTPCode code, MIME mime_type)
{
	std::string headers = buildStatusLine(code);
	headers += "Content-Type: " + std::string(MIME::toString(mime_type)) + "\r\n";
	headers += "Transfer-Encoding: chunked\r\n";
	headers += "Connection: keep-alive\r\n\r\n";
    
	connection.sendData(headers);
}

void Response::sendChunk(Connection& connection, const std::string& body)
{
	if (body.empty()) 
		return;

	std::stringstream ss;
	ss << std::hex << body.size();

	std::string chunk = ss.str() + "\r\n" + body + "\r\n";
	connection.sendData(chunk);
}

void Response::sendEndChunks(Connection& connection)
{
	std::string end = "0\r\n\r\n";
	connection.sendData(end);
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
