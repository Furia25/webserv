/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Address.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 20:31:42 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/06 18:23:53 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Address.hpp"

Address::Address()
{
	std::memset(&this->data, 0, sizeof(data));
	addr_len = 0;
}

Address::Address(const std::string &host, const std::string &service, const struct addrinfo *raw_addr) :
	family(raw_addr->ai_family), type(raw_addr->ai_socktype), protocol(raw_addr->ai_protocol),
	flags(raw_addr->ai_flags), host(host), service(service)
	
{
	std::memcpy(&this->data, raw_addr->ai_addr, raw_addr->ai_addrlen);
	addr_len = raw_addr->ai_addrlen;
}

Address::Address(const Address &other)
{
	*this = other;
}

Address::~Address() {}

Address &Address::operator=(const Address &other)
{
	if (this == &other)
		return (*this);
	this->host = other.host;
	this->data = other.data;
	this->addr_len = other.addr_len;
	this->family = other.family;
	this->flags = other.flags;
	this->service = other.service;
	this->type = other.type;
	this->protocol = other.protocol;
	return (*this);
}

std::string Address::toString(bool host, bool service) const
{
	char	hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

	int result = getnameinfo(
		(struct sockaddr*)&this->data, this->addr_len,
		hbuf, host ? sizeof(hbuf) : 0,
		sbuf, service ? sizeof(sbuf) : 0,
		NI_NUMERICHOST | NI_NUMERICSERV
	);
	if (result != 0)
		return std::string("Unknown");
	std::stringstream	ss;
	if (host) ss << hbuf;
	if (host && service) ss << ':';
	if (service) ss << sbuf;
	return ss.str();
}

bool Address::isIPv6(void) const
{
	return this->family == AF_INET6;
}

const std::string &Address::getHost(void) const { return this->host; }

const std::string& Address::getService(void)const { return this->service; }

const sockaddr *Address::getSockAddr(void) const { return reinterpret_cast<const struct sockaddr*>(&data);}

socklen_t Address::getAddrLen(void) const { return this->addr_len; }

int Address::getFamily(void) const { return this->family; }

int Address::getType(void) const { return this->type; }

int Address::getProtocol(void) const { return this->protocol; }

int Address::getFlags(void) const { return this->flags; }

std::ostream &operator<<(std::ostream &os, const Address &addr)
{
	os << addr.toString();
	return (os);
}
