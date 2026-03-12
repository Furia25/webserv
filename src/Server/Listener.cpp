/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Listener.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/10 19:05:01 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/12 17:53:55 by vdurand          ###   ########.fr       */
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
	socket.listen(MAX_PENDING_CONNECTION);
}

Listener::~Listener() {}

void Listener::handleEvent(TCPServer& server, uint32_t events)
{
	/*if (events & EPOLLERR || events & EPOLLHUP)
	{	
		server.recoverListener(*this);
		return ;
	}*/
	if (events & EPOLLIN)
	{
		while (true)
		{
			Connection *client_connection = NULL;
			try {
				client_connection = new Connection(this->getSocket());
			}
			catch (const SocketException& e) {delete client_connection; break;}
			catch (const std::exception& e) {delete client_connection; throw;}
			server.registerConnection(client_connection);
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
