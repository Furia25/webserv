/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Listener.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/10 19:05:01 by vdurand           #+#    #+#             */
/*   Updated: 2026/05/22 04:22:05 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server/Listener.hpp"
#include "Server/Connection.hpp"
#include "Server/TCPServer.hpp"

Listener::Listener(const char *host, const char *service)
{
	std::vector<Address>	addresses = AddressResolver::resolve(host, service);
	socket.open(SOCK_STREAM, AF_INET);
	socket.setReuseAddr(true);
	socket.bind(addresses);
	socket.setIOBlocking(false);
	socket.listen(MAX_PENDING_CONNECTION);
}

Listener::~Listener() {}

void Listener::handleEvent(TCPServer& server, uint32_t events)
{
	if (events & EPOLLERR || events & EPOLLHUP)
		throw std::runtime_error("Error on listener, unable to continue");

	if (!(events & EPOLLIN))
		return ;
	
	while (server.actualConnections < MAX_CLIENTS)
	{
		Connection	*client_connection = NULL;
		void		*ptr = server.connectionPool.acquire();

		try
		{
			client_connection = new (ptr) Connection(server, this->getSocket(), this->getAddress().getPort());
		}
		catch (const SocketException& e)
		{
			server.connectionPool.releaseRaw(ptr);
			if (e.getErrorCode() == EAGAIN)
				break ;
			Logger::ERROR() << "Accept errored :" << e.what();
			break ;
		}
		catch (...) { server.connectionPool.releaseRaw(ptr); throw ; }
		server.registerConnection(client_connection);
		#if HTTP_DEBUG == true
		Logger::DEBUG() << "Connection established: " << this->getSocket().getAddress()
				<< " <-> "<< client_connection->getSocket().getAddress();
		#endif
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
