/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Listener.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/10 19:05:01 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/16 18:04:53 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server/Listener.hpp"
#include "Server/Connection.hpp"
#include "Server/TCPServer.hpp"

Listener::Listener(const char *host, const char *service)
{
	std::vector<Address>	addresses = AddressResolver::resolve(host, service);
	socket.open(SOCK_STREAM, AF_INET);
	socket.bind(addresses);
	socket.setIOBlocking(false);
	socket.listen(MAX_PENDING_CONNECTION);
}

Listener::~Listener() {}

void Listener::handleEvent(TCPServer& server, uint32_t events)
{
	if (events & EPOLLERR || events & EPOLLHUP)
	{	
		server.recoverListener(*this);
		return ;
	}
	if (events & EPOLLIN)
	{
		while (server.actual_connections < MAX_CLIENTS)
		{
			Connection *client_connection = NULL;
			try {
				client_connection = new Connection(this->getSocket());
			}
			catch (const SocketException& e) {break;}
			catch (const std::exception& e) {throw;}
			server.registerConnection(client_connection);
			Logger::INFO() << "Connection established: Listener " << this->getSocket().getAddress() << " -> " << client_connection->getSocket().getAddress();
		}
	}
}

Socket &Listener::getSocket(void)
{
	return this->socket;
}

const Socket &Listener::getSocket(void) const
{
	return this->socket;
}

const Address &Listener::getAddress(void) const
{
	return this->socket.getAddress();
}
