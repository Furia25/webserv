/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/08 14:50:07 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/31 11:33:10 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server/Connection.hpp"
#include "Server/TCPServer.hpp"

size_t Connection::last_id = 0;

Connection::Connection(TCPServer& server, Socket& server_socket) : server(server), bytes_sended(0), bytes_received(0), state(CONNECTED), alarmTimeout(this, timeoutCallback)
{
	this->client_socket.accept(server_socket);
	this->read_buffer.reserve(READ_BUFFER_DEFAULT_SIZE);
	this->write_buffer.reserve(WRITE_BUFFER_DEFAULT_SIZE);
	this->id = Connection::last_id;
	Connection::last_id++;
	TCPServer::AlarmManager.reschedule(this->alarmTimeout, ABSOLUTE_TIMEOUT);
}

Connection::~Connection() {}

void Connection::handleEvent(TCPServer &server, uint32_t events)
{
	(void) server;
	if (events & EPOLLHUP && events & EPOLLRDHUP)
		this->setDeletable();
	if (this->state == CLOSING && this->write_buffer.size() == 0 && this->read_buffer.size() == 0)
		this->setDeletable();
	if (events & EPOLLIN && this->state == CONNECTED)
	{
		TCPServer::AlarmManager.reschedule(this->alarmTimeout, ABSOLUTE_TIMEOUT);
		this->handleRead();
		server.getHandler().onDataReceived(*this);
	}
	if (events & EPOLLOUT && this->state != DELETABLE)
	{
		TCPServer::AlarmManager.reschedule(this->alarmTimeout, CLOSING_TIMEOUT);
		this->handleWrite();
	}
}

void Connection::timeout(Alarm<Connection *>& alarm)
{
	(void) alarm;
	this->setDeletable();
}

void Connection::setDeletable(void) 
{
	this->state = DELETABLE;
	this->server.dropConnection(this);
}

void Connection::handleRead(void)
{
	if (this->read_buffer.size() > READ_LIMIT)
	{
		this->setDeletable();
		return ;
	}

	uint8_t	buffer[READ_SIZE];

	ssize_t n = this->client_socket.receive(buffer, sizeof(buffer));
	if (n > 0)
		this->read_buffer.insert(this->read_buffer.end(), buffer, buffer + n);
	else if (n == 0)
	{
		TCPServer::AlarmManager.reschedule(this->alarmTimeout, CLOSING_TIMEOUT);
		this->state = CLOSING;
	}
	/*Logger::DEBUG() << this->read_buffer;*/
}

void Connection::handleWrite(void)
{
	if (this->bytes_sended >= this->write_buffer.size())
		this->clearWriteBuffer();

	const uint8_t *buffer_ptr = &this->write_buffer[this->bytes_sended];
	ssize_t remaining = this->write_buffer.size() - this->bytes_sended;

	ssize_t sent = this->client_socket.send(buffer_ptr, remaining);
	if (sent > 0)
		this->bytes_sended += sent;

	if (this->bytes_sended >= this->write_buffer.size())
		this->clearWriteBuffer();
}

void Connection::sendData(const uint8_t *data, size_t len)
{
	this->server.setPollEvent(*this, CONNECTION_EVENTS | EPOLLOUT);
	this->write_buffer.insert(this->write_buffer.end(), data, data + len);
}

void Connection::sendData(const std::string& data)
{
	this->server.setPollEvent(*this, CONNECTION_EVENTS | EPOLLOUT);
	this->write_buffer.insert(this->write_buffer.end(), data.begin(), data.end());
}

void Connection::consumeReadData(size_t n)
{
	this->bytes_received += n;
	if (this->bytes_received >= this->read_buffer.size())
		this->clearReadBuffer();
}

const uint8_t *Connection::getReadBufferPtr() const
{
	return &this->read_buffer[this->bytes_received];
}

const std::vector<uint8_t> &Connection::getReadVector() const
{
	return this->read_buffer;
}

size_t Connection::getReadBufferSize() const
{
	return this->read_buffer.size() - this->bytes_received;
}

void Connection::clearReadBuffer()
{
	this->read_buffer.clear();
	this->bytes_received = 0;
}

void Connection::clearWriteBuffer()
{
	this->server.setPollEvent(*this, CONNECTION_EVENTS);
	this->write_buffer.clear();
	this->bytes_sended = 0;
}

void Connection::compactReadBuffer()
{
	this->clearReadBuffer();
	this->read_buffer.resize(READ_BUFFER_DEFAULT_SIZE);
}

void Connection::compactWriteBuffer()
{
	this->clearWriteBuffer();
	this->write_buffer.resize(WRITE_BUFFER_DEFAULT_SIZE);
}

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

Connection::State Connection::getState(void) const
{
	return this->state;
}

const Address &Connection::getAddress(void) const
{
	return this->client_socket.getAddress();
}

const char *Connection::getStateString(State state)
{
	switch (state) {
		#define X(el) \
			case el: \
				return #el;
				_CONNECTION_STATES
			default:
				return "Unknown";
		#undef X
	}
}

void timeoutCallback(Alarm<Connection *> &alarm, Connection* connection)
{
	connection->timeout(alarm);
}

bool operator==(const Connection &lhs, const Connection &rhs)
{
	return lhs.id == rhs.id;
}
