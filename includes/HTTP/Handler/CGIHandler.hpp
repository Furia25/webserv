/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 17:08:05 by vdurand           #+#    #+#             */
/*   Updated: 2026/05/18 03:35:01 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _CGIHANDLER_H
# define _CGIHANDLER_H

# include <vector>

# include "Config/Config.hpp"
# include "HTTP/AHandler.hpp"
# include "HTTP/Response.hpp"

class CGIHandler : public AHandler, IEpollHandler
{
public:
	CGIHandler(
		HTTPHandler& handler,
		Connection& connection,
		const Request& request,
		Body& body,
		const Router::RouteResult& route_result,
		HTTPCode status_code = HTTPCode::OK)
	: AHandler(handler, connection, request, body, route_result, status_code), 
	CGIConfig(static_cast<const Config::CGIConfig&>(*route_result.route)), registered(false) {};

	~CGIHandler();

	void	onCreation();
	void	onExecute() {};

	void	initPaths();
	void	initEnvironment();
	void	setEnv(const std::string& key, const std::string& value);

private:
	const Config::CGIConfig&	CGIConfig;
	std::vector<std::string>	envFlat;

	std::string					pathInfo;
	std::string					scriptName;
	std::string					scriptFilename;

	bool						registered;

	CGIHandler(const CGIHandler& other);
	CGIHandler&	operator=(const CGIHandler& other);
};


#endif // _CGIHANDLER_H
