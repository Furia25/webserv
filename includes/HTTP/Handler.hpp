/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Handler.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 18:43:47 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/20 20:10:57 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _HANDLER_H
# define _HANDLER_H

# include "HTTP/IHandler.hpp"
# include "HTTP/Request.hpp"
# include "Server/Connection.hpp"

class StaticHandler: public IHandler
{
public:
    
    StatichHandler(Request &request, Connection &connection, std::string &path, RouteConfig &route);
    void handle();
};

class CgiHandler: public IHandler
{
public:
    Response handle(const Request &req, const RouteConfig &config);
};

#endif // _HANDLER_H