/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 18:25:39 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/05 17:31:48 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _RESPONSE_H
# define _RESPONSE_H

# include "Server/Connection.hpp"
# include "HTTP/HttpTypes.hpp"

class Response
{
public:
    static void buildErrorResponse(Connection& connection, HTTPCode code);

    static void buildRawResponse(Connection& connection, HTTPCode code, MIME mime_type, const std::string& body);

    static void buildEmptyResponse(Connection& connection, HTTPCode code);
    
    static void buildFileHeaderResponse(Connection& connection, HTTPCode code, MIME mime_type, size_t fileSize);
    
    static void sendEndChunks(Connection& connection);
    
    static void sendChunkedHeader(Connection& connection, HTTPCode code, MIME mime_type);

    static void sendChunk(Connection& connection, const std::string& body);
    
    static void sendBodyChunk(Connection& connection, const uint8_t* data, size_t len);
};

#endif // _RESPONSE_H