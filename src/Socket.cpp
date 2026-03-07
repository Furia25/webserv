/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 20:21:29 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/07 17:32:33 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"

Socket::Socket() : socket_fd(-1), state(CLOSED) {}

Socket::~Socket()
{
	if (this->socket_fd != -1)
		close(this->socket_fd);
}

void Socket::open(int type, int domain, int protocol)
{
	if (this->state != CLOSED)
		throw SocketException("Socket already opened");
	int	temp_fd = socket(domain, type, protocol);
	if (temp_fd == -1)
		throw SocketException(strerror(errno));
	this->socket_fd = temp_fd;
	this->state = OPEN;
	this->setReuseAddr(true);
}

void Socket::open_dual(int type, int protocol)
{
	this->open(type, AF_INET6, protocol);
	this->setDualStack(true);
}

void Socket::bind(const Address& addr)
{
	if (this->state != OPEN)
		throw SocketException("Bind", ESOCK_INVALID);
	errno = 0;
	if (::bind(socket_fd, addr.getSockAddr(), addr.getAddrLen()) == -1)
		throw SocketException("Bind", strerror(errno));
	this->state = BOUND;
	this->address = addr;
}

void Socket::bind(const std::vector<Address>& addresses)
{
	if (this->state != OPEN)
		throw SocketException("Bind", ESOCK_INVALID);
	std::vector<Address>::const_iterator	it;
	for (it = addresses.begin(); it != addresses.end(); it++)
	{
		const Address& addr = *it;
		errno = 0;
		if (::bind(socket_fd, addr.getSockAddr(), addr.getAddrLen()) == 0)
		{
			this->address = addr;
			break ;
		}
		SOCKET_DEBUG_ERROR;
	}
	if (it == addresses.end())
		throw SocketException("Bind", strerror(errno));
	this->state = BOUND;
}

void Socket::connect(const Address &addr)
{
	if (this->state != OPEN)
		throw SocketException("Connect", ESOCK_INVALID);
	errno = 0;
	if (::connect(this->socket_fd, addr.getSockAddr(), addr.getAddrLen()) == -1)
		throw SocketException("Connect", strerror(errno));
	this->state = CONNECTED;
	this->address = addr;
}

void Socket::connect(const std::vector<Address>& addresses)
{
	if (this->state != OPEN)
		throw SocketException("Connect", ESOCK_INVALID);
	std::vector<Address>::const_iterator	it;
	for (it = addresses.begin(); it != addresses.end(); it++)
	{
		const Address& addr = *it;
		errno = 0;
		if (::connect(socket_fd, addr.getSockAddr(), addr.getAddrLen()) == 0)
		{
			this->address = addr;
			break ;
		}
		SOCKET_DEBUG_ERROR;
	}
	if (it == addresses.end())
		throw SocketException("Connect", strerror(errno));
	this->state = CONNECTED;
}

void Socket::setDualStack(bool enable)
{
	int val = enable ? 0 : 1;
	this->setOption(IPPROTO_IPV6, IPV6_V6ONLY, val);
}

const char* Socket::stateToString() const
{
	switch (this->state)
	{
		case CLOSED:	return "CLOSED";
		case BOUND:		return "BOUND";
		case OPEN:		return "OPEN";
		case CONNECTED:	return "CONNECTED";
		case LISTENING:	return "LISTENING";
		default:		return "UNKNOWN";
	} 
}

std::string Socket::toString(int error) const
{
	std::stringstream ss;
	ss << "Socket [" << this->socket_fd << "] | State: " << stateToString();

	switch (this->state)
	{
		case BOUND: ss << " | Local: " << this->address.toString(); break;
		case CONNECTED: ss << " | Peer: " << this->address.toString(); break;
		case LISTENING: ss << " | Listening on: " << this->address.getService(); break;	
		default: break;
	}
	if (errno != 0)
		ss << " | Error : " << strerror(error);
	return ss.str();
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

Socket::State Socket::getState() const { return this->state;  }

const Address &Socket::getAddress() const { return this->address; }

std::ostream &operator<<(std::ostream &os, const Socket &socket)
{
	os << socket.toString();
	return (os);
}
