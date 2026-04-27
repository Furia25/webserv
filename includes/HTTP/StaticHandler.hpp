/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StaticHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 18:43:47 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/27 14:27:28 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _HANDLER_H
# define _HANDLER_H

# include "HTTP/IHandler.hpp"
# include "HTTP/Request.hpp"
# include "Server/Connection.hpp"
# include "Config/Config.hpp"
# include <string>

enum State
{
	INIT,
	SEND_HEADERS,
	SEND_BODY,
	FINISHED
};

class StaticHandler: public IHandler
{
public:
	StaticHandler(const Request &req, Connection &connection, const std::string& physical_path, const Config::ServerConfig* host, HTTPCode statusCode = HTTPCode::OK);
	virtual ~StaticHandler();

	bool execute();

private:
	void handleError(HTTPCode code);

	Request						request;
	Connection&					connection;
	std::string					physical_path;
	const Config::ServerConfig	*host;
	bool						isFinished;
	State						state;
	std::ifstream				file_stream;
	HTTPCode					statusCode;
};

#endif // _HANDLER_H
