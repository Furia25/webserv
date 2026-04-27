/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Router.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 11:01:45 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/27 11:35:37 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _ROUTER_H
# define _ROUTER_H

# include "Config/Config.hpp"
# include "HTTP/Request.hpp"
# include "HTTP/HttpTypes.hpp"
# include "HTTP/RouteResult.hpp"

class Router 
{
private:
	const Config::AppConfig& _config;
	std::string extractHost(const Request& req) const;

public:
	Router(const Config::AppConfig& config);
	RouteResult resolve(const Request& req) const;
};

#endif // _ROUTER_H