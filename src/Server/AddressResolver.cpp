/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AddressResolver.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/06 02:55:13 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/10 17:13:38 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server/AddressResolver.hpp"

static std::vector<Address> init_address(const std::string& host, const std::string& service, int type)
{
	std::vector<Address>	addresses;
	struct addrinfo hints, *res, *node;
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = type;
	if (host == "")
		hints.ai_flags = AI_PASSIVE;
	int error_code = getaddrinfo(host == "" ? NULL : host.c_str(), service.c_str(), &hints, &res);
	if (error_code != 0)
		throw std::runtime_error(gai_strerror(error_code));
	node = res;
	while (node)
	{
		Address addr(host, service, node);
		addresses.push_back(addr);
		node = node->ai_next;
	}
	freeaddrinfo(res);
	return addresses;
}

std::vector<Address> AddressResolver::resolve(const std::string& host, const std::string& service, int type)
{
	return init_address(host, service, type);
}

std::vector<Address> AddressResolver::resolve(const char *host, const char *service, int type)
{
	return init_address(std::string(host), std::string(service), type);
}
