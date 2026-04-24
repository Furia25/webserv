/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TCPServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 17:43:15 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/23 14:09:06 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _TCPSERVER_H
# define _TCPSERVER_H

# include <map>
# include <vector>
# include <cstring>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netdb.h>
# include <stdexcept>
# include <cerrno>
# include <sstream>
# include <csignal>
# include <sys/epoll.h>

# include "Config/Config.hpp"
# include "EnumClass.hpp"
# include "Address.hpp"
# include "AddressResolver.hpp"
# include "Socket.hpp"
# include "IRequestHandler.hpp"
# include "Listener.hpp"
# include "Connection.hpp"
# include "Logger.hpp"
# include "Utils/HashMap.hpp"
# include "Utils/HashedTimingWheel.hpp"

# define MAX_CLIENTS	1024
# define MAX_EVENTS	512
# define MAX_PENDING_CONNECTION	10
# define EPOLL_TIMEOUT	1000

# define LISTENER_EVENTS	EPOLLIN | EPOLLERR | EPOLLHUP
# define CONNECTION_EVENTS	EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLRDHUP

class TCPServer
{
public:
	TCPServer(const Config::EngineConfig& config);
	~TCPServer();

	void				run(void);

	void				openListener(const std::string& host, const std::string& service);
	void				openListener(const std::string& host, unsigned int port);
	void				openListener(const char *host, const char *service);

	void				cleanConnections(void);

	void				bindHandler(IRequestHandler& handler);
	IRequestHandler&	getHandler(void);

	static void			tickCallback(void *instance);

	static	HashedTimingWheel<EPOLL_TIMEOUT> AlarmManager;

	friend class Connection;
	friend class Listener;
protected:
	void	clearListeners();
	void	clearConnections();
private:
	size_t						actual_connections;
	int							epoll_fd;
	IRequestHandler				*handler;
	std::vector<Listener*>		listeners;
	HashMap<int, Connection*>	connections;
	std::vector<Connection *>	deletable_connections;

	const Config::EngineConfig&			engineConfig;

	void	recoverListener(Listener& listener);

	void	setPollEvent(IEpollHandler& event_handler, uint32_t events);
	void	addPollEvent(IEpollHandler& event_handler, uint32_t events);
	void	removePollEvent(IEpollHandler& event_handler);
	void	registerConnection(Connection *connection);
	void	dropConnection(Connection* connection);
};

#endif // _TCPSERVER_H