/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TCPServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 19:03:54 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/05 20:18:50 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "TCPServer.hpp"

TCPServer::~TCPServer()
{
	
}

void TCPServer::openListener(const std::string &ip, unsigned int port)
{
	std::stringstream	ss;
	ss << port;
	this->openListener(ip == "" ? NULL : ip.c_str(), ss.str().c_str());
}

void TCPServer::openListener(const char *ip, const char *service)
{
	struct addrinfo hints, *res;
	std::memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if (ip == NULL)
		hints.ai_flags = AI_PASSIVE;

	int error_code = getaddrinfo(ip, service, &hints, &res);
	if (error_code != 0)
		throw std::runtime_error(gai_strerror(error_code));

	int	socket_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (socket_fd == -1)
		goto error;
	if (bind(socket_fd, res->ai_addr, res->ai_addrlen) == -1)
		goto error;

	freeaddrinfo(res);
	if (listen(socket_fd, MAX_PENDING_CONNECTION) == -1)
		throw std::runtime_error(strerror(errno));
	this->sockets.push_back(socket_fd);
	
	error:
		freeaddrinfo(res);
		throw std::runtime_error(strerror(errno));
}

