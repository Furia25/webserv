/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RedirectHandler.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antoine <antoine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/11 23:19:27 by antoine           #+#    #+#             */
/*   Updated: 2026/05/11 23:46:39 by antoine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/Handler/RedirectHandler.hpp"

void	RedirectHandler::onExecute()
{
	const Config::RedirectConfig* conf = static_cast<const Config::RedirectConfig*>(routeConfig);

	std::string destination = conf->redirect_location;
    HTTPCode finalCode = conf->status;

	std::stringstream ss;
	ss << "HTTP/1.1 " << static_cast<int>(finalCode) << " " << HTTPCode::toString(finalCode) << "\r\n";
	ss << "Location: " << destination << "\r\n";
	ss << "Content-Length: 0\r\n";
	ss << "Connection: close\r\n\r\n";

	connection.sendData(ss.str());

	this->setFinished();
	connection.setClosing();

	Logger::INFO() << "Redirection " << static_cast<int>(finalCode) << " to " << destination;
}
