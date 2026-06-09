/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ErrorHandler.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/28 14:39:49 by antbonin          #+#    #+#             */
/*   Updated: 2026/06/09 20:08:36 by vdurand          ###   ########.fr       */
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
