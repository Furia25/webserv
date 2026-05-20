/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 17:08:05 by vdurand           #+#    #+#             */
/*   Updated: 2026/05/21 00:33:00 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _CGIHANDLER_H
# define _CGIHANDLER_H

# include <vector>

# include "Config/Config.hpp"
# include "HTTP/AHandler.hpp"
# include "HTTP/Response.hpp"
# include "HTTP/CGIFactory.hpp"

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
	CGIConfig(static_cast<const Config::CGIConfig&>(*route_result.route)),
	timeout(false),
	alarmTimeout(this, timeoutCallback),
	isBinary(false),
	registered(false),
	childPID(-1)
	{
		this->pipeIn[0] = -1;
		this->pipeIn[1] = -1;

		this->pipeOut[0] = -1;
		this->pipeOut[1] = -1;
	};

	~CGIHandler();

	void	onCreation();

	void	onExecute();
	void	handleEvent(TCPServer& server, uint32_t events);

	void	initPaths();
	void	initEnvironment();
	void	initProcessVariables(const char *argv[3], std::vector<const char *>& envp);

private:
	const Config::CGIConfig&	CGIConfig;

	CGIResponseParser			CGIParser;

	std::vector<std::string>	envFlat;

	bool						timeout;
	Alarm<CGIHandler *>			alarmTimeout;

	std::string					pathInfo;
	std::string					scriptName;
	std::string					scriptFilename;

	bool						isBinary;
	std::string					interpreter;

	bool						registered;
	int							pipeIn[2];
	int							pipeOut[2];
	pid_t						childPID;

	std::string					line;

	friend void	timeoutCallback(Alarm<CGIHandler *>& alarm, CGIHandler *handler);

	CGIHandler(const CGIHandler& other);
	CGIHandler&	operator=(const CGIHandler& other);
};

void	timeoutCallback(Alarm<CGIHandler *>& alarm, CGIHandler *handler) { handler->timeout = true; };

#endif _CGIHANDLER_H
