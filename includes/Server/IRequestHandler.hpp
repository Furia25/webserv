/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IRequestHandler.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 17:54:59 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/15 18:02:39 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _IREQUESTHANDLER_H
# define _IREQUESTHANDLER_H

# include <vector>
# include <stdint.h>

# include "Connection.hpp"

class IRequestHandler
{
public:
	virtual void	onDataReceived(Connection& connection) = 0;
	virtual void	onConnection(Connection& connection) = 0;
	virtual void	onDisconnection(Connection& connection) = 0;
	virtual void	onError(Connection& connection) = 0;
};

#endif // _IREQUESTHANDLER_H