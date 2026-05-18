/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TCPServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 19:03:54 by vdurand           #+#    #+#             */
/*   Updated: 2026/05/18 22:38:13 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server/TCPServer.hpp"

HashedTimingWheel<EPOLL_TIMEOUT> TCPServer::AlarmManager;

volatile sig_atomic_t	g_running = true;

static void signal_handler(int signum)
{
	(void) signum;
	g_running = false;
}

TCPServer::TCPServer(const Config::EngineConfig& engine_config) : connectionPool(4946), engineConfig(engine_config)
{
	this->startTime = 0;
	this->actualConnections = 0;
	this->epollfd = ::epoll_create(MAX_CLIENTS);
	if (this->epollfd == -1)
		throw std::runtime_error("Unable to init epoll :" + std::string(strerror(errno)));
	void *ptr = this->connectionPool.acquire();
	this->connectionPool.releaseRaw(ptr);
}

TCPServer::~TCPServer()
{
	this->clearListeners();
	this->cleanConnections();
	this->clearConnections();
	close(this->epollfd);
}

void TCPServer::run(void)
{
	if (::signal(SIGINT, signal_handler) == SIG_ERR)
		throw std::runtime_error("Couldn't set handler for signal");

	this->startTime = time(NULL);

	std::vector<epoll_event>	events_vec;
	events_vec.reserve(this->engineConfig.max_events);

	epoll_event	*events = events_vec.data();

	while (g_running)
	{
		timestamp_ms next_timeout = AlarmManager.next_timeout_ms();
		int timeout = next_timeout == 0 ? EPOLL_TIMEOUT : next_timeout;
		int n = epoll_wait(this->epollfd, events, this->engineConfig.max_events, timeout);
		if (n < 0)
		{
			if (errno == EINTR)
				continue ;
			Logger::FATAL() << "Epoll wait returned -1" << strerror(errno);
			break ;
		}
		for (int i = 0; i < n; ++i)
		{
			IEpollHandler *event_handler = reinterpret_cast<IEpollHandler *>(events[i].data.ptr);
			event_handler->handleEvent(*this, events[i].events);
		}
		Logger::tick();
		AlarmManager.tick();
		this->cleanConnections();
	}
}

void TCPServer::openListener(const std::string &host, const std::string &service)
{
	this->openListener(host == "" ? NULL : host.c_str(), service.c_str());
}

void TCPServer::openListener(const std::string& host, port_t port)
{
	std::stringstream	ss;
	ss << port;
	this->openListener(host == "" ? NULL : host.c_str(), ss.str().c_str());
}

void TCPServer::openListener(const char *host, const char *service)
{
	Listener *listener = new Listener(host, service);
	Logger::INFO() << "Listening on " << listener->getAddress();
	this->listeners.push_back(listener);
	this->addPollEvent(*listener, listener->getSocket().getFd(), LISTENER_EVENTS);
}

void TCPServer::cleanConnections(void)
{
	for (std::vector<Connection *>::iterator it = this->deletableConnections.begin(); it != this->deletableConnections.end(); ++it)
		this->connectionPool.release(*it);
	this->deletableConnections.clear();
}

void TCPServer::registerConnection(Connection *connection)
{
	this->connections.insert(connection->getSocket().getFd(), connection);
	this->handler->onConnection(*connection);
	this->addPollEvent(*connection, connection->getSocket().getFd(), CONNECTION_EVENTS);
	this->actualConnections++;
}

void TCPServer::dropConnection(Connection *connection)
{
	int	fd = connection->getSocket().getFd();

	this->connections.erase(fd);
	this->handler->onDisconnection(*connection);
	this->removePollEvent(*connection, fd);
	this->deletableConnections.push_back(connection);
	#if HTTP_DEBUG == true
	Logger::DEBUG() << "Connection dropped: Client " << connection->getSocket().getAddress();
	#endif
	this->actualConnections--;
}

/*TODO : LOGGING FOR BETTER RECOVERY BECAUSE ITS A SUICIDE FUNCTION FOR THE LISTENER
 REALLY REALLY UNSAFE IDK WHY I DO THAT, I AM A FOOL
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
	this->removePollEvent(listener, );
	if (on_heap)
		delete &listener;
	this->openListener(address.getHost(), address.getService());
}*/

void TCPServer::setPollEvent(IEpollHandler &event_handler, int fd, uint32_t events)
{
	epoll_event ev;
	ev.events = events;
	ev.data.ptr = &event_handler;
	errno = 0;
	if (epoll_ctl(this->epollfd, EPOLL_CTL_MOD, fd, &ev) == -1)
		throw std::runtime_error("Unable to add a polling event: " + std::string(strerror(errno)));
}

void TCPServer::addPollEvent(IEpollHandler &event_handler, int fd, uint32_t events){
	epoll_event ev;
	ev.events = events;
	ev.data.ptr = &event_handler;
	errno = 0;
	if (epoll_ctl(this->epollfd, EPOLL_CTL_ADD, fd, &ev) == -1)
		throw std::runtime_error("Unable to add a polling event: " + std::string(strerror(errno)));
}

void TCPServer::removePollEvent(IEpollHandler &event_handler, int fd)
{
	errno = 0;
	int result = epoll_ctl(this->epollfd, EPOLL_CTL_DEL, fd, NULL);
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
	for (HashMap<int, Connection *>::iterator it = this->connections.begin(); it != this->connections.end(); ++it)
		this->connectionPool.release(it->second);
	this->connections.clear();
}

void TCPServer::bindHandler(IRequestHandler &handler)
{
	this->handler = &handler;
}

IRequestHandler& TCPServer::getHandler(void)
{
	return *this->handler;
}

void TCPServer::tickCallback(void *instance)
{
	TCPServer *server = static_cast<TCPServer *>(instance);
	Logger::DEBUG() << "Heartbeat: Server is running...";
	LogMessage debug = Logger::DEBUG();
	debug << "Connections: ";
	for (HashMap<int, Connection *>::iterator it = server->connections.begin(); it != server->connections.end(); ++it)
	{
		debug << (*(*it).second);
		if (it != server->connections.end())
			debug << ", ";
	}
}
