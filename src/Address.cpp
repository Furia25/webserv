/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Address.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 20:31:42 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/05 20:59:52 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Address.hpp"

static void init_address(const char *host, const char *service)
{
	struct addrinfo hints, *res;
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if (host == NULL)
		hints.ai_flags = AI_PASSIVE;
}

Address::Address(const std::string &host, int service)
{
}

Address::Address(const char *host, const char *service)
{
	init_address(host, service);
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
