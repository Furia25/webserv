/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Handler.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 18:43:47 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/16 18:45:41 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _HANDLER_H
# define _HANDLER_H

# include "IHandler.hpp"

class StaticHandler: public IHandler
{
public:
    Response handle(const Request &req, const RouteConfig &config);
};

class CgiHandler: public IHandler
{
public:
    Response handle(const Request &req, const RouteConfig &config);
};

#endif // _HANDLER_H