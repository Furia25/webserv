/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Address.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antoine <antoine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 20:31:42 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/06 15:03:01 by antoine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Address.hpp"

void Address::init_address(const char *host, const char *service)
{
	struct addrinfo hints, *res;
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if (host == NULL)
		hints.ai_flags = AI_PASSIVE;
	int error_code = getaddrinfo(host, service, &hints, &res);
	if (error_code != 0)
		throw std::runtime_error(gai_strerror(error_code));
	this->family = res->ai_family;
	this->protocol = res->ai_protocol;
	this->type = res->ai_socktype;
	this->flags = res->ai_flags;
	std::memcpy(&this->data, res->ai_addr, res->ai_addrlen);
	this->addr_len = res->ai_addrlen;
	freeaddrinfo(res);
}

Address::Address(const std::string &host, int service)
{
	std::stringstream	ss;
	ss << service;
	this->host = std::string(host);
	this->service = service;
	this->init_address(host == "" ? NULL : host.c_str(), ss.str().c_str());
}

Address::Address(const char *host, const char *service)
{
	this->host = std::string(host);
	errno = 0;
	this->service = strtod(service, NULL);
	if (errno != 0)
		throw std::invalid_argument(strerror(errno));
	this->init_address(host, service);
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
	this->addr_len = other.addr_len;
	this->data = other.data;
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
	if (host)
		ss << host;
	if (host && service)
		ss << ':';
	if (service)
		ss << service;
	return ss.str();
}

bool Address::isIPv6(void) const
{
	return this->family == AF_INET6;
}

const std::string &Address::getHost(void) const { return this->host; }

const int Address::getService(void)const { return this->service; }

const sockaddr *Address::getSockAddr(void) const { return (sockaddr *)this->data; }

socklen_t Address::getAddrLen(void) const { return this->addr_len; }

int Address::getFamily(void) const { return this->family; }

int Address::getType(void) const { return this->type; }

int Address::getProtocol(void) const { return this->protocol; }

int Address::getFlags(void) const { return this->flags; }

std::ostream &operator<<(std::ostream &os, const Address &addr)
{
	os << addr.toString();
}
