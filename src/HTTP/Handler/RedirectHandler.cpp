/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RedirectHandler.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/11 23:19:27 by antoine           #+#    #+#             */
/*   Updated: 2026/05/15 03:48:24 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/Handler/RedirectHandler.hpp"

void	RedirectHandler::onCreation()
{
	response.sendStatusLine(this->redirectConfig.status)
		.sendDefaults(this->request, *this->routeResult.route)
		.sendHeader("Location", this->redirectConfig.redirect_location)
		.sendEnd();
	this->setFinished();
}
