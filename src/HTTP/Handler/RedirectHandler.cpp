/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RedirectHandler.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/11 23:19:27 by antoine           #+#    #+#             */
/*   Updated: 2026/05/13 02:55:34 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/Handler/RedirectHandler.hpp"

void	RedirectHandler::onExecute()
{
	std::string destination = this->redirectConfig.redirect_location;
    HTTPCode finalCode = this->redirectConfig.status;

	/*TODO*/
	std::stringstream ss;
	ss << HTTP_VERSION " " << static_cast<int>(finalCode) << " " << HTTPCode::toString(finalCode) << "\r\n";
	ss << "Location: " << destination << "\r\n";
	ss << "Content-Length: 0\r\n";
	ss << "Connection: close\r\n\r\n";

	connection.sendData(ss.str());

	this->setFinished();
	connection.setClosing();

	Logger::INFO() << "Redirection " << static_cast<int>(finalCode) << " to " << destination;
}
