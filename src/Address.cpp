/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Address.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 20:31:42 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/08 16:00:51 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Address.hpp"

static std::pair<std::string, std::string> get_hostname_info(const sockaddr *addr, socklen_t addr_len, bool numeric);

Address::Address()
{
	std::memset(&this->data, 0, sizeof(data));
	addr_len = 0;
}

Address::Address(const std::string &host, const std::string &service, const struct addrinfo *raw_addr) :
	domain(raw_addr->ai_family), type(raw_addr->ai_socktype), protocol(raw_addr->ai_protocol),
	flags(raw_addr->ai_flags), host(host), service(service)
	
{
	std::memcpy(&this->data, raw_addr->ai_addr, raw_addr->ai_addrlen);
	addr_len = raw_addr->ai_addrlen;
	if (host == "" || service == "")
	{
		std::pair<std::string, std::string> info = get_hostname_info(
			raw_addr->ai_addr,
			raw_addr->ai_addrlen,
			NUMERICAL_HOSTNAME_RESOLUTION);
		if (host == "")
			this->host = info.first;
		if (service == "")
			this->service = info.second;
	}
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
	this->domain = other.domain;
	this->flags = other.flags;
	this->service = other.service;
	this->type = other.type;
	this->protocol = other.protocol;
	return (*this);
}

static std::pair<std::string, std::string> get_hostname_info(const sockaddr *addr, socklen_t addr_len, bool numeric)
{
	char	host[NI_MAXHOST];
	char	service[NI_MAXSERV];

	int result = ::getnameinfo(
		addr, addr_len,
		host, sizeof(host),
		service, sizeof(service),
		numeric ? NI_NUMERICHOST | NI_NUMERICSERV : 0
	);
	if (result != 0)
		return std::make_pair("Unknown", "Unknown");
	return std::make_pair(std::string(host), std::string(service));
}

std::string Address::toString(bool host, bool service) const
{
	std::stringstream	ss;
	if (host) ss << this->host;
	if (host && service) ss << ':';
	if (service) ss << this->service;
	return ss.str();
}

bool Address::isIPv6(void) const
{
	return this->domain == AF_INET6;
}

const std::string &Address::getHost(void) const
{
	return this->host;
}

const std::string& Address::getService(void) const
{
	return this->service;
}

const sockaddr *Address::getSockAddr(void) const { return reinterpret_cast<const struct sockaddr*>(&data);}

socklen_t Address::getAddrLen(void) const { return this->addr_len; }

int Address::getDomain(void) const { return this->domain; }

int Address::getType(void) const { return this->type; }

int Address::getProtocol(void) const { return this->protocol; }

int Address::getFlags(void) const { return this->flags; }

std::ostream &operator<<(std::ostream &os, const Address &addr)
{
	os << addr.toString();
	return (os);
}
