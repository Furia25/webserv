/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/08 14:50:07 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/08 16:16:14 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Connection.hpp"

size_t Connection::last_id = 0;

Connection::Connection() : bytes_sended(0), bytes_received(0)
{
	this->id = Connection::last_id;
	Connection::last_id++;
}

Connection::~Connection() {}

size_t Connection::getClientID(void) const
{
	return this->id;
}

Socket &Connection::getSocket(void)
{
	return this->client_socket;
}

const Socket &Connection::getSocket(void) const
{
	return this->client_socket;
}

bool operator==(const Connection &lhs, const Connection &rhs)
{
	return lhs.id == rhs.id;
}
