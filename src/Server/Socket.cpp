/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 20:21:29 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/10 18:47:09 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server/Socket.hpp"

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
		throw SocketException("Open", strerror(errno));
	this->socket_fd = temp_fd;
	this->state = OPEN;
	this->type = type;
	this->domain = domain;
	this->protocol = protocol;
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

void Socket::listen(unsigned int backlog)
{
	if (this->state != BOUND)
		throw SocketException("Listen", ESOCK_INVALID);
	errno = 0;
	if (::listen(this->socket_fd, backlog) == -1)
		throw SocketException("Listen", strerror(errno));
	this->state = LISTENING;
}

void Socket::accept(Socket& server)
{
	if (server.state != LISTENING)
		throw SocketException("Accept", ESOCK_INVALID);
	sockaddr_storage	addr_storage;
	sockaddr			*addr_ptr = reinterpret_cast<struct sockaddr *>(&addr_storage);
	socklen_t			temp_len = sizeof(sockaddr_storage);
	int temp_fd = ::accept(server.socket_fd, addr_ptr, &temp_len);
	if (temp_fd == -1)
		throw SocketException("Accept", strerror(errno));
	addrinfo addr_info;
	addr_info.ai_family = temp_len == 16 ? AF_INET : AF_INET6;
	addr_info.ai_addr = addr_ptr;
	addr_info.ai_addrlen = temp_len;
	addr_info.ai_protocol = server.protocol;
	addr_info.ai_socktype = server.type;
	this->address = Address("", "", &addr_info);
	this->type = server.type;
	this->protocol = server.protocol;
	this->state = CONNECTED;
	this->domain = addr_info.ai_family;
}

ssize_t Socket::receive(void *buffer, size_t size, int flags)
{
	return ::recv(this->socket_fd, buffer, size, flags);
}

ssize_t Socket::send(const void *buffer, size_t size, int flags)
{
	return ::send(this->socket_fd, buffer, size, flags);
}

void Socket::setDualStack(bool enable)
{
	int val = enable ? 0 : 1;
	this->setOption(IPPROTO_IPV6, IPV6_V6ONLY, val);
}

void Socket::setIOBlocking(bool blocking)
{
	int flags = this->fcntl(F_GETFL, 0);
	flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
	if (this->fcntl(F_SETFL, flags) != 0)
		throw SocketException("fctnl", strerror(errno));
}

int Socket::fcntl(int command, long arg)
{
	int result = ::fcntl(this->socket_fd, command, arg);
	if (result == -1)
		throw SocketException("fctnl", strerror(errno));
	return result;
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
	return os;
}

int Socket::getFd() const
{
	return this->socket_fd;
}
