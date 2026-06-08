/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RouterException.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/08 14:45:23 by antbonin          #+#    #+#             */
/*   Updated: 2026/06/08 14:48:42 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _ROUTEREXCEPTION_H
# define _ROUTEREXCEPTION_H

# include "HTTP/HTTPTypes.hpp"
# include <stdexcept>

class RouterException : public std::exception 
{
private:
	HTTPCode code;
public:
	RouterException(HTTPCode code);
	virtual ~RouterException() throw();
	HTTPCode getCode() const;
};

#endif // _ROUTEREXCEPTION_H