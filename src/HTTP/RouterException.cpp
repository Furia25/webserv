/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RouterException.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/08 14:47:13 by antbonin          #+#    #+#             */
/*   Updated: 2026/06/08 14:57:32 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/RouterException.hpp"

RouterException::RouterException(HTTPCode code) : code(code)
{
}

RouterException::~RouterException() throw()
{
}

HTTPCode RouterException::getCode() const
{
	return this->code;
}