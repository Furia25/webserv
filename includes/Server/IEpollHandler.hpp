/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IEpollHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/10 19:01:25 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/13 18:03:57 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _IEPOLLHANDLER_H
# define _IEPOLLHANDLER_H

# include "Socket.hpp"
# include "Logger.hpp"
# include <sys/epoll.h>

class TCPServer;

class IEpollHandler
{
public:
	virtual void			handleEvent(TCPServer& server, uint32_t events) = 0;
	virtual Socket&			getSocket(void) = 0;
	virtual const Socket&	getSocket(void) const = 0;
	virtual const Address&	getAddress(void) const = 0;
protected:
private:
};

#endif // _IEPOLLHANDLER_H