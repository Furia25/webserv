/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Handler.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antoine <antoine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 18:00:00 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/20 23:18:39 by antoine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HTTP/Handler.hpp"

// =========================================================================
// StaticHandler
// =========================================================================

StaticHandler::StaticHandler(const Request &request, const RouteConfig &route, Connection &connection)
    : request(request), route(route), connection(connection), isFinished(false)
{
}

StaticHandler::~StaticHandler()
{
}

bool StaticHandler::execute()
{
    isFinished = true;
    return _isFinished;
}

void StaticHandler::handle(const Request &req, const RouteConfig &config, Connection &connection)
{
    (void)req;
    (void)config;
    (void)connection;
}

CgiHandler::CgiHandler(const Request &request, const RouteConfig &route, Connection &connection)
    : request(request), route(route), connection(connection)
{
}

CgiHandler::~CgiHandler()
{
}

bool CgiHandler::execute()
{
    return true;
}

void CgiHandler::handle(const Request &req, const RouteConfig &config, Connection &connection)
{
    (void)req;
    (void)config;
    (void)connection;
}
