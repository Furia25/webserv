/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorHandler.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 14:39:49 by antbonin          #+#    #+#             */
/*   Updated: 2026/06/08 16:23:11 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/Handler/ErrorHandler.hpp"
# include "Utils/FileSystem.hpp"
# include <string>

void ErrorHandler::onCreation() 
{
	throw HTTPException(this->statusCode);
}

void ErrorHandler::onExecute()
{
	throw HTTPException(this->statusCode);
}
