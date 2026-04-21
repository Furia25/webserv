/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 18:25:39 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/21 15:56:17 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _RESPONSE_H
# define _RESPONSE_H

# include "Server/Connection.hpp"

class Response
{
public:
    static void buildErrorResponse(Connection& connection, HTTPCode code);

    static void buildRawResponse(Connection& connection, HTTPCode code, const std::string& contentType, const std::string& body);

    static void buildEmptyResponse(Connection& connection, HTTPCode code);
    
    static void buildFileHeaderResponse(Connection& connection, HTTPCode code, const std::string& contentType, size_t fileSize);
    
    static void sendBodyChunk(Connection& connection, const uint8_t* data, size_t len);
};

#endif // _RESPONSE_H