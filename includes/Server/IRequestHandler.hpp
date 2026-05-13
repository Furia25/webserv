/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IRequestHandler.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 17:54:59 by vdurand           #+#    #+#             */
/*   Updated: 2026/05/13 02:25:01 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _IRequestHandler_H
# define _IRequestHandler_H

# include <vector>
# include <stdint.h>

# include "Connection.hpp"

class IRequestHandler
{
public:
	virtual void	onDataReceived(Connection& connection) = 0;
	virtual void	onConnection(Connection& connection) = 0;
	virtual void	onDisconnection(Connection& connection) = 0;
	virtual void	onError(Connection& connection, uint32_t error_event) = 0;
};

#endif // _IRequestHandler_H