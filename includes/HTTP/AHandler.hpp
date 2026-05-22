/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AHandler.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 18:39:26 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/22 05:01:11 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# ifndef _IHANDLER_H
# define _IHANDLER_H

# include "HTTP/Router.hpp"
# include "HTTP/HTTPTypes.hpp"
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

	void	setFinished() { this->finished = true;};
	bool	isFinished() const { return this->finished; };

	void	sendFullDefaultError();

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
			statusCode(status_code),
			physicalPath(route_result.fullPath),
			response(connection), state(INIT), errored(false), first(true) {};

	const HTTPHandler&				handler;
	Connection&						connection;
	const Request&					request;
	Body&							body;
	const Router::RouteResult		routeResult;
	bool							fileHeaderSent;
	bool							finished;
	HTTPCode						statusCode;
	std::string						physicalPath;
	Response						response;

private:
	enum State
	{
		INIT,
		SEND_HEADERS,
		SEND_BODY,
		SEND_DEFAULT_ERROR,
		FINISHED
	};

	FileReader	fileReader;
	State		state;
	bool		errored;
	bool		first;

	void	handleError();
	void	initError();

	AHandler(const AHandler& other);
	AHandler& operator=(const AHandler& other);
};

#endif // _IHANDLER_H