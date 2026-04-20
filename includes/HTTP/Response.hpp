/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 18:25:39 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/20 19:20:07 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _RESPONSE_H
# define _RESPONSE_H

# include "Server/Connection.hpp"

class Response
{
public:
    Response();
    Response(const Response& other);
    ~Response();
    Response& operator=(const Response& other);
    static Response buildErrorResponse(Connection &connection, int statusCode);
protected:
private:
};

#endif // _RESPONSE_H