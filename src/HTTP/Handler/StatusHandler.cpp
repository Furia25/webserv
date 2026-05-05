/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StatusHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 10:50:35 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/05 18:24:05 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/Handler/StatusHandler.hpp"
# include "HTTP/Response.hpp"
# include "HTTP/HttpTypes.hpp"
# include "Utils/FileSystem.hpp"
# include "Server/TCPServer.hpp"

void StatusHandler::onExecute()
{
	const TCPServer&	server	= connection.getServer();
	time_t				now = time(NULL);
	
	std::ostream& os = Logger::getStream();
	long uptime = (long)(now - server.getStartTime());

	std::ostringstream json;
	json << "{\n" << " \"status\": \"ok\",\n";
	if (this->statusConfig.server_info)
	{
		json << " \"server\": {\n"
			<< " \"name\": \"webserv\",\n"
			<< " \"version\": \"1.0.0\",\n"
			<< " \"uptime_seconds\": " << uptime << "\n"
			<< " },\n";
	}
	if (this->statusConfig.connection_info)
	{
		json << " \"connections\": {\n"
			<< " \"active\": " << server.getConnectionsCount() << ",\n"
			<< " \"total_handled\": " << server.getTotalConnections() << "\n"
			<< " },\n";
	}
	if (this->statusConfig.request_info)
	{
		json << " \"requests\": {\n"
			<< " \"total\": " << totalHandled << ",\n"
			<< " \"success\": " << (totalHandled - errors) << ",\n"
			<< " \"errors\": " << errors << "\n"
			<< " },\n";
	}
	if (this->statusConfig.timestamp)
	{
		struct tm		datetime;
		char			timestr[50];
		struct timeval	tv;

		gettimeofday(&tv, NULL);
		localtime_r(&tv.tv_sec, &datetime);
		std::strftime(timestr, sizeof(timestr), "%Y-%m-%dT%H:%M:%SZ", &datetime);
		json << " \"timestamp\": \"" << timestr << "\"\n";
	}
	json << "}";

	Response::buildRawResponse(this->connection, HTTPCode::OK, MIME::json, json.str());
}
