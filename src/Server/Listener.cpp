/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Listener.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/10 19:05:01 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/10 19:14:21 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Listener.hpp"

Listener::Listener(const char *host, const char *service)
{
	std::vector<Address>	addresses = AddressResolver::resolve(host, service);
	socket.open(SOCK_STREAM, AF_INET);
	socket.bind(addresses);
	socket.listen(MAX_PENDING_CONNECTION);
}

Listener::~Listener() {}

void Listener::handleEvent(TCPServer &server, uint32_t events)
{
}

Socket &Listener::getSocket(void)
{
	return this->socket;
}

const Socket &Listener::getSocket(void) const
{
	return this->socket;
}
