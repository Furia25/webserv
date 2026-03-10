/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TCPServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 17:43:15 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/10 19:15:22 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _TCPSERVER_H
# define _TCPSERVER_H

# include <vector>
# include <cstring>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netdb.h>
# include <stdexcept>
# include <cerrno>
# include <sstream>
# include <map>
# include <sys/epoll.h>

# include "Address.hpp"
# include "AddressResolver.hpp"
# include "Socket.hpp"
# include "IRequestHandler.hpp"
# include "Listener.hpp"

# define MAX_EVENTS	512
# define MAX_PENDING_CONNECTION	10
# define EPOLL_TIMEOUT	-1

# define LISTENER_EVENTS	EPOLLIN | EPOLLERR | EPOLLHUP

class TCPServer
{
public:
	TCPServer();
	~TCPServer();

	void	run(void);

	void	openListener(const std::string& host, const std::string& service);
	void	openListener(const std::string& host, unsigned int port);
	void	openListener(const char *host, const char *service);

	void	addPollEvent(IEpollHandler& event_handler, uint32_t events);
	
	void	bindHandler(IRequestHandler& handler);
protected:
	void	clearListeners();
	void	clearConnections();
private:
	int									epoll_fd;
	IRequestHandler						*handler;
	std::vector<Listener *>				listeners;
	std::map<int, Connection *>			connections;
};

#endif // _TCPSERVER_H