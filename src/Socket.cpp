/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 20:21:29 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/06 04:33:59 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"

Socket::Socket() : socket_fd(-1), state(CLOSED) {}

Socket::~Socket()
{
	if (this->socket_fd != -1)
		close(this->socket_fd);
}

void Socket::open(int type, int protocol, int domain)
{
	if (this->state != CLOSED)
		throw SocketException("Socket already opened");
	int	temp_fd = socket(domain, type, protocol);
	if (temp_fd == -1)
		throw SocketException(strerror(errno));
	this->socket_fd = temp_fd;
	this->state = OPEN;
}

void Socket::open_dual(int type, int protocol)
{
	this->open(type, protocol, AF_INET6);
	this->setDualStack(true);
}

void Socket::setDualStack(bool enable)
{
	int val = enable ? 0 : 1;
	this->setOption(IPPROTO_IPV6, IPV6_V6ONLY, val);
}

void Socket::setReuseAddr(bool enable)
{
	int val = enable ? 1 : 0;
	this->setOption(SOL_SOCKET, SO_REUSEADDR, val);
}

void Socket::setNoDelay(bool enable)
{
	int val = enable ? 1 : 0;
	setOption(IPPROTO_TCP, TCP_NODELAY, val);
}

Socket::State Socket::getState() const
{
	return this->state;
}
