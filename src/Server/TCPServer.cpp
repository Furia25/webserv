/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TCPServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 19:03:54 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/13 18:45:41 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server/TCPServer.hpp"

volatile sig_atomic_t	g_running = true;

static void signal_handler(int signum)
{
	(void) signum;
	g_running = false;
}

TCPServer::TCPServer()
{
	this->epoll_fd = ::epoll_create(100);
	if (this->epoll_fd == -1)
		throw std::runtime_error("Unable to init epoll :" + std::string(strerror(errno)));
}

TCPServer::~TCPServer()
{
	this->clearListeners();
	this->clearConnections();
	close(this->epoll_fd);
}

void TCPServer::run(void)
{
	std::signal(SIGINT, signal_handler);
	epoll_event	events[MAX_EVENTS];

	while (g_running)
	{
		int n = epoll_wait(this->epoll_fd, events, MAX_EVENTS, EPOLL_TIMEOUT);
		for (int i = 0; i < n; ++i)
		{
			IEpollHandler *event_handler = static_cast<IEpollHandler *>(events[i].data.ptr);
			event_handler->handleEvent(*this, events[i].events);
		}
		this->cleanConnections();
		Logger::tick();
	}
}

void TCPServer::openListener(const std::string &host, const std::string &service)
{
	this->openListener(host == "" ? NULL : host.c_str(), service.c_str());
}

void TCPServer::openListener(const std::string& host, unsigned int port)
{
	std::stringstream	ss;
	ss << port;
	this->openListener(host == "" ? NULL : host.c_str(), ss.str().c_str());
}

void TCPServer::openListener(const char *host, const char *service)
{
	Listener *listener = new Listener(host, service);
	this->listeners.push_back(listener);
	this->addPollEvent(*listener, LISTENER_EVENTS);
}

void TCPServer::registerConnection(Connection *connection)
{
	this->connections.push_back(connection);
	this->addPollEvent(*connection, CONNECTION_EVENTS);
}

void TCPServer::dropConnection(Connection *connection)
{
	this->handler->onDisconnection(*connection);
	this->removePollEvent(*connection);
}

void TCPServer::cleanConnections(void)
{
	std::vector<Connection *>::iterator it = this->connections.begin();
	for (; it != this->connections.end(); ++it)
	{
		Connection *connection = *it;
		if (connection->getState() == Connection::DELETABLE)
		{
			this->connections.erase(it);
			this->dropConnection(connection);
		}
	}
}

/*TODO : LOGGING FOR BETTER RECOVERY BECAUSE ITS A SUICIDE FUNCTION FOR THE LISTENER
 REALLY REALLY UNSAFE IDK WHY I DO THAT, I AM A FOOL
 */
void TCPServer::recoverListener(Listener& listener)
{
	bool		on_heap = false;
	Socket&		listener_socket = listener.getSocket();

	std::vector<Listener *>::iterator it = this->listeners.begin();
	for (; it != this->listeners.end(); ++it)
	{
		if ((*it)->getSocket().getFd() == listener_socket.getFd())
		{
			this->listeners.erase(it);
			on_heap = true;
			break ;
		}
	}
	const Address	address = listener_socket.getAddress();
	this->removePollEvent(listener);
	if (on_heap)
		delete &listener;
	this->openListener(address.getHost(), address.getService());
}

void TCPServer::addPollEvent(IEpollHandler &event_handler, uint32_t events)
{
	epoll_event ev;
	ev.events = events;
	ev.data.ptr = &event_handler;
	errno = 0;
	if (epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, event_handler.getSocket().getFd(), &ev) == -1)
		throw std::runtime_error("Unable to add a polling event: " + std::string(strerror(errno)));
}

void TCPServer::removePollEvent(IEpollHandler &event_handler)
{
	errno = 0;
	int result = epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, event_handler.getSocket().getFd(), NULL);
	if (result == -1 && !(errno == ENOENT || errno == EBADF))
		throw std::runtime_error("Unable to remove a polling event: " + std::string(strerror(errno)));
}

void TCPServer::clearListeners()
{
	for (std::vector<Listener *>::iterator it = this->listeners.begin(); it != this->listeners.end(); ++it)
		delete *it;
	this->listeners.clear();
}

void TCPServer::clearConnections()
{
	for (std::vector<Connection *>::iterator it = this->connections.begin(); it != this->connections.end(); ++it)
		delete *it;
	this->connections.clear();
}

void TCPServer::bindHandler(IRequestHandler &handler)
{
	this->handler = &handler;
}

IRequestHandler &TCPServer::getHandler(void)
{
	return *this->handler;
}

void TCPServer::tickCallback(void *instance)
{
	TCPServer *server = static_cast<TCPServer *>(instance);
	Logger::DEBUG() << "Heartbeat: Server is running...";
	LogMessage debug = Logger::DEBUG();
	debug << "Connections: ";
	for (std::vector<Connection *>::const_iterator it = server->connections.begin(); it != server->connections.end(); it++)
	{
		debug << (*(*it));
		if (it != server->connections.end())
			debug << ", ";
	}
}
