/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AddressResolver.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/06 02:55:13 by vdurand           #+#    #+#             */
/*   Updated: 2026/06/09 15:08:44 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Server/AddressResolver.hpp"

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

port_t AddressResolver::getPortFromService(const std::string& service_name)
{
	struct addrinfo hints, *res;
	port_t port = 0;

	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo(NULL, service_name.c_str(), &hints, &res) == 0)
	{
		if (res->ai_family == AF_INET)
		{
			struct sockaddr_in* ipv4 = reinterpret_cast<struct sockaddr_in*>(res->ai_addr);
			port = ntohs(ipv4->sin_port);
		}
		else if (res->ai_family == AF_INET6)
		{
			struct sockaddr_in6* ipv6 = reinterpret_cast<struct sockaddr_in6*>(res->ai_addr);
			port = ntohs(ipv6->sin6_port);
		}
		freeaddrinfo(res);
	}
	return port;
}
