/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IEpollHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/10 19:01:25 by vdurand           #+#    #+#             */
/*   Updated: 2026/05/18 04:02:50 by vdurand          ###   ########.fr       */
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
protected:
private:
};

#endif // _IEPOLLHANDLER_H