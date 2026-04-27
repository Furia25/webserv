/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RouteResult.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 11:00:34 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/27 11:01:28 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _ROUTERESULT_H
# define _ROUTERESULT_H

# include "Config/Config.hpp"

struct RouteResult {
    const Config::ServerConfig* host;
    const Config::RouteConfig* route;
    std::string                 physicalPath;
    HTTPCode                    errorCode;
    bool                        success;

    RouteResult() : host(NULL), route(NULL), errorCode(HTTPCode::OK), success(false) {}
};

#endif // _ROUTERESULT_H