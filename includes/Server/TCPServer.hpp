/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TCPServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 17:43:15 by vdurand           #+#    #+#             */
/*   Updated: 2026/06/12 18:01:18 by vdurand          ###   ########.fr       */
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
# include <csignal>
# include <sys/epoll.h>

# include "Config/Config.hpp"
# include "EnumClass.hpp"

# include "Server/Address.hpp"
# include "Server/AddressResolver.hpp"
# include "Server/Socket.hpp"
# include "Server/IRequestHandler.hpp"
# include "Server/Listener.hpp"
# include "Server/Connection.hpp"

# include "Logger.hpp"

# include "Utils/HashMap.hpp"
# include "Utils/HashedTimingWheel.hpp"
# include "Utils/FreeList.hpp"

# define MAX_CLIENTS	16384
# define MAX_PENDING_CONNECTION	4096

# define EPOLL_TIMEOUT	1000

# define LISTENER_EVENTS	EPOLLIN | EPOLLERR | EPOLLHUP
# define CONNECTION_EVENTS	EPOLLIN | EPOLLERR | EPOLLHUP | EPOLLRDHUP

# define TCP_EPOLL_DEBUG	false

class TCPServer
{
public:
	TCPServer(const Config::EngineConfig& config);
	~TCPServer();

	void				run(void);

	void				openListener(const std::string& host, const std::string& service);
	void				openListener(const std::string& host, port_t port);
	void				openListener(const char *host, const char *service);

	void				cleanConnections(void);

	void				bindHandler(IRequestHandler& handler);
	IRequestHandler&	getHandler(void);

	time_t				getStartTime(void) const { return this->startTime; };
	size_t				getTotalConnections(void) const { return Connection::last_id; };
	size_t				getConnectionsCount(void) const { return this->actualConnections; };

	void				setPollEvent(IEpollHandler& event_handler, int fd, uint32_t events);
	void				addPollEvent(IEpollHandler& event_handler, int fd, uint32_t events);
	void				removePollEvent(IEpollHandler& event_handler, int fd);

	static void			tickCallback(void *instance);

	static	HashedTimingWheel<EPOLL_TIMEOUT> AlarmManager;

	friend class Connection;
	friend class Listener;

protected:
	void	clearListeners();
	void	clearConnections();

private:
	time_t						startTime;
	size_t						actualConnections;
	int							epollfd;
	IRequestHandler				*handler;

	std::vector<Listener*>		listeners;

	HashMap<int, Connection*>	connections;
	std::vector<Connection *>	deletableConnections;
	FreeList<Connection>		connectionPool;

	std::vector<IEpollHandler*>	freeHooks;

	const Config::EngineConfig&	engineConfig;

	void	recoverListener(Listener& listener);

	void	registerConnection(Connection *connection);
	void	dropConnection(Connection* connection);
};

class ForkException { };

#endif // _TCPSERVER_H