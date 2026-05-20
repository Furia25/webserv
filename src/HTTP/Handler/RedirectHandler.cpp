/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RedirectHandler.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/11 23:19:27 by antoine           #+#    #+#             */
/*   Updated: 2026/05/21 00:35:10 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/Handler/RedirectHandler.hpp"

void	RedirectHandler::onExecute()
{
	response.sendStatusLine(this->redirectConfig.status)
		.sendDefaults(request, this->routeResult.route)
		.sendContentLength(0)
		.sendHeader("Location", this->redirectConfig.redirect_location)
		.sendEnd();
	this->setFinished();
}
