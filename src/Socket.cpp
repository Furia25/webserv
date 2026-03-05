/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 20:21:29 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/05 20:26:56 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"

Socket::Socket() : socket_fd(-1) {}
Socket::Socket(int fd) : socket_fd(fd) {}

Socket::~Socket()
{
	if (this->socket_fd != -1)
		close(this->socket_fd);
}

int Socket::getFD(void) const
{
	return this->socket_fd;
}
