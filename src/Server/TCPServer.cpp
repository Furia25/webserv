/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TCPServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 19:03:54 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/10 19:15:31 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server/TCPServer.hpp"

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
	epoll_event	events[MAX_EVENTS];

	while (true)
	{
		int n = epoll_wait(this->epoll_fd, events, MAX_EVENTS, EPOLL_TIMEOUT);
		for (int i = 0; i < n; i++)
		{
			const Socket *socket = static_cast<Socket *>(events[i].data.ptr);
			if (this->connections.find(socket->getFd()) == this->connections.end())
			{
				//Listeners
				
			}
			else
			{
				//Connections
				
			}
		}
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

void TCPServer::addPollEvent(IEpollHandler &event_handler, uint32_t events)
{
	epoll_event ev;
	ev.events = events;
	ev.data.ptr = &event_handler;
	epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, event_handler.getSocket().getFd(), &ev);
}

void TCPServer::clearListeners()
{
	for (std::vector<Listener *>::iterator it = this->listeners.begin(); it != this->listeners.end(); ++it)
		delete *it;
	this->listeners.clear();
}

void TCPServer::clearConnections()
{
	for (std::map<int, Connection *>::iterator it = this->connections.begin(); it != this->connections.end(); ++it)
		delete (*it).second;
	this->connections.clear();
}

void TCPServer::bindHandler(IRequestHandler &handler)
{
	this->handler = &handler;
}
