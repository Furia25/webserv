/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Handler.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antoine <antoine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 18:43:47 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/20 23:17:57 by antoine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _HANDLER_H
# define _HANDLER_H

# include "HTTP/IHandler.hpp"
# include "HTTP/Request.hpp"
# include "Server/Connection.hpp"
# include "Config/Config.hpp"
# include <string>

class StaticHandler: public IHandler
{
public:
    StaticHandler(const Request &request, const RouteConfig &route, Connection &connection);
    virtual ~StaticHandler();

    bool execute();
    void handle(const Request &req, const RouteConfig &config, Connection &connection);

private:
    const Request&     request;
    const RouteConfig& route;
    Connection&        connection;
    std::string        filePath;
    bool               isFinished;
};

class CgiHandler: public IHandler
{
public:
    CgiHandler(const Request &request, const RouteConfig &route, Connection &connection);
    virtual ~CgiHandler();

    bool execute();
    void handle(const Request &req, const RouteConfig &config, Connection &connection);
    
private:
    const Request&     request;
    const RouteConfig& route;
    Connection&        connection;
};

#endif // _HANDLER_H
