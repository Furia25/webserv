/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IHandler.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 18:39:26 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/20 19:23:06 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


# ifndef _IHANDLER_H
# define _IHANDLER_H
# include "Request.hpp"
# include "../Config/Config.hpp"
# include "../Server/Connection.hpp"
# include "Response.hpp"
# include "IJob.hpp"

class IHandler : public IJob
{
public:
    virtual void handle(const Request &req, const RouteConfig &config, Connection &connection) = 0;
    
    virtual ~IHandler() {};
};

#endif // _IHANDLER_H