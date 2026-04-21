/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/21 15:59:45 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/21 16:01:12 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/Response.hpp"
# include "HTTP/HttpTypes.hpp"
# include <sstream>

static std::string itoa(int n)
{
    std::stringstream ss;
    ss << n;
    return ss.str();
}

static std::string buildStatusLine(HTTPCode code)
{
    return "HTTP/1.1 " + itoa(static_cast<int>(code)) + " " + HTTPCode::toString(code) + "\r\n";
}

void Response::buildErrorResponse(Connection& connection, HTTPCode code)
{
    std::string body = "<html><head><title>" + itoa(static_cast<int>(code)) + " " + HTTPCode::toString(code) + "</title></head><body><h1>" + itoa(static_cast<int>(code)) + " " + HTTPCode::toString(code) + "</h1></body></html>";
    
    std::string response = buildStatusLine(code);
    response += "Content-Type: text/html\r\n";
    response += "Content-Length: " + itoa(body.length()) + "\r\n";
    response += "Connection: close\r\n\r\n";
    response += body;
    
    connection.sendData(response);
}

void Response::buildRawResponse(Connection& connection, HTTPCode code, const std::string& contentType, const std::string& body)
{
    std::string response = buildStatusLine(code);
    response += "Content-Type: " + contentType + "\r\n";
    response += "Content-Length: " + itoa(body.length()) + "\r\n";
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

void Response::buildFileHeaderResponse(Connection& connection, HTTPCode code, const std::string& contentType, size_t fileSize)
{
    std::string response = buildStatusLine(code);
    response += "Content-Type: " + contentType + "\r\n";
    response += "Content-Length: " + itoa(fileSize) + "\r\n";
    response += "Connection: close\r\n\r\n";
    
    connection.sendData(response);
}

void Response::sendBodyChunk(Connection& connection, const uint8_t* data, size_t len)
{
    connection.sendData(data, len);
}
