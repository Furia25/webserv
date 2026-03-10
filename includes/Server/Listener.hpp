/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Listener.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/10 19:03:15 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/10 19:07:31 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _LISTENER_H
# define _LISTENER_H

# include "Socket.hpp"
# include "IEpollHandler.hpp"

class Listener : public IEpollHandler
{
public:
	Listener(const char *host, const char *service);
	~Listener();

	void			handleEvent(TCPServer &server, uint32_t events);

	Socket&			getSocket(void);
	const Socket&	getSocket(void) const;
protected:
private:
	Socket			socket;
};

#endif // _LISTENER_H