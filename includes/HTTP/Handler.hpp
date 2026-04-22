/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Handler.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 18:43:47 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/22 11:50:17 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _HANDLER_H
# define _HANDLER_H

# include "HTTP/IHandler.hpp"
# include "HTTP/Request.hpp"
# include "Server/Connection.hpp"
# include "Config/Config.hpp"
# include <string>

enum State
{
    INIT,
    SEND_HEADERS,
    SEND_BODY,
    FINISHED
};

class StaticHandler: public IHandler
{
public:
    StaticHandler(const Request &req, const RouteConfig *route, Connection &connection, const std::string& physical_path);
    virtual ~StaticHandler();

    bool execute();

private:
    Request            request;
    const RouteConfig* route;
    Connection&        connection;
    std::string        physical_path;
    bool               isFinished;
    State              state;
    std::ifstream      file_stream;
};

// class CgiHandler: public IHandler
// {
// public:
//     CgiHandler(const Request &req, const RouteConfig *route, Connection &connection, const std::string& physical_path);
//     virtual ~CgiHandler();

//     bool execute();
    
// private:
//     Request            request;
//     const RouteConfig* route;
//     Connection&        connection;
//     std::string        physical_path;
// };

#endif // _HANDLER_H
