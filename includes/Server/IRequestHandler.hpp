/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IRequestHandler.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 17:54:59 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/17 16:27:20 by antbonin         ###   ########.fr       */
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