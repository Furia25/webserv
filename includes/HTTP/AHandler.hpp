/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AHandler.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antoine <antoine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 18:39:26 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/12 22:17:46 by antoine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# ifndef _IHANDLER_H
# define _IHANDLER_H

# include "HTTP/HttpTypes.hpp"
# include "HTTP/Request.hpp"
# include "Config/Config.hpp"
# include "Server/Connection.hpp"
# include "HTTP/Response.hpp"
# include "Server/IJob.hpp"
# include "Utils/FileReader.hpp"

class HTTPHandler;
class Body;

class AHandler : public IJob
{
public:
	virtual ~AHandler() {};
	bool	execute();
	virtual void onExecute() = 0;
	virtual void onCreation() {};

	void	setFinished() { this->finished = true; };
	bool	isFinished() const { return this->finished; };

protected:
	AHandler(
		HTTPHandler& handler,
		Connection& connection,
		const Request& request,
		Body& body,
		const Config::ServerConfig *host_config,
		const Config::RouteConfig *route_config,
		const std::string& physical_path,
		HTTPCode status_code = HTTPCode::OK) :
			handler(handler),
			fileHeaderSent(false),
			connection(connection),
			request(request),
			body(body),
			hostConfig(host_config),
			routeConfig(route_config),
			physicalPath(physical_path),
			finished(false),
			statusCode(status_code) {};

	const HTTPHandler&				handler;
	bool							fileHeaderSent;
	Connection&						connection;
	const Request&					request;
	Body&							body;
	const Config::ServerConfig* 	hostConfig;
	const Config::RouteConfig*		routeConfig;
	std::string						physicalPath;
	bool							finished;
	HTTPCode						statusCode;

private:
	void	initError();
};

#endif // _IHANDLER_H