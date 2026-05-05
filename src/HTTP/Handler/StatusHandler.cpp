/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   StatusHandler.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/05 18:57:13 by vdurand           #+#    #+#             */
/*   Updated: 2026/05/05 19:04:49 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/HTTPHandler.hpp"
# include "HTTP/Handler/StatusHandler.hpp"
# include "HTTP/Response.hpp"
# include "HTTP/HttpTypes.hpp"
# include "Utils/FileSystem.hpp"
# include "Server/TCPServer.hpp"

void StatusHandler::onExecute()
{
	const TCPServer& server = connection.getServer();
	std::vector<std::string> sections;
	std::stringstream ss;

	sections.push_back("\"status\": \"ok\"");

	if (this->statusConfig.server_info)
	{
		time_t now = time(NULL);
		long uptime = (long)(now - server.getStartTime());
		
		ss.str(""); ss.clear();
		ss << "\"server\": {\n"
			<< "	\"name\": \"" << this->hostConfig->name << "\",\n"
			<< "	\"version\": \"" SERV_VERSION "\",\n"
			<< "	\"uptime_seconds\": " << uptime << "\n"
			<< "}";
		sections.push_back(ss.str());
	}

	if (this->statusConfig.connection_info)
	{
		ss.str(""); ss.clear();
		ss << "\"connections\": {\n"
		   << "	\"active\": " << server.getConnectionsCount() << ",\n"
		   << "	\"total_handled\": " << server.getTotalConnections() << "\n"
		   << "}";
		sections.push_back(ss.str());
	}

	if (this->statusConfig.request_info)
	{
		ss.str(""); ss.clear();
		ss << "\"requests\": {\n"
		   << "	\"total\": " << this->handler.getTotalRequests() << "\n"
		   << "}";
		sections.push_back(ss.str());
	}

	if (this->statusConfig.timestamp)
	{
		struct tm datetime;
		char timestr[50];
		time_t now = time(NULL);
		localtime_r(&now, &datetime);
		std::strftime(timestr, sizeof(timestr), "%Y-%m-%dT%H:%M:%SZ", &datetime);
		
		sections.push_back("\"timestamp\": \"" + std::string(timestr) + "\"");
	}

	std::string json = "{\n";
	for (size_t i = 0; i < sections.size(); ++i)
	{
		json += "  " + sections[i];
		if (i < sections.size() - 1)
			json += ",";
		json += "\n";
	}
	json += "}";

	Response::buildRawResponse(this->connection, HTTPCode::OK, MIME::json, json);
}
