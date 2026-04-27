/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 18:25:39 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/27 17:57:42 by vdurand          ###   ########.fr       */
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
    
    static void sendBodyChunk(Connection& connection, const uint8_t* data, size_t len);
};

#endif // _RESPONSE_H