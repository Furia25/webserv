/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AHandler.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 18:39:26 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/13 02:39:12 by vdurand          ###   ########.fr       */
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
# include "HTTP/Utils/FileReader.hpp"

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
		const Router::RouteResult& route_result,
		HTTPCode status_code = HTTPCode::OK) :
			handler(handler),
			connection(connection),
			request(request),
			body(body),
			routeResult(route_result),
			fileHeaderSent(false),
			finished(false),
			statusCode(status_code) {};

	const HTTPHandler&				handler;
	Connection&						connection;
	const Request&					request;
	Body&							body;
	const Router::RouteResult&		routeResult;
	bool							fileHeaderSent;
	bool							finished;
	HTTPCode						statusCode;

private:
	void	initError();
};

#endif // _IHANDLER_H