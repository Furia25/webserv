/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/08 14:50:07 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/24 16:29:37 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server/Connection.hpp"
#include "Server/TCPServer.hpp"
#include "HTTP/IJob.hpp"

size_t Connection::last_id = 0;

Connection::Connection(TCPServer& server, Socket& server_socket) : server(server), bytes_sended(0),
	bytes_received(0), state(CONNECTED), alarmTimeout(this, timeoutCallback), engineConfig(server.engineConfig), actual_job(NULL)
{
	this->client_socket.accept(server_socket);
	this->read_buffer.reserve(4096);
	this->write_buffer.reserve(WRITE_BUFFER_DEFAULT_SIZE);
	this->id = Connection::last_id;
	Connection::last_id++;
	TCPServer::AlarmManager.reschedule(this->alarmTimeout, this->engineConfig.max_timeout);
}

Connection::~Connection()
{
	TCPServer::AlarmManager.cancel(this->alarmTimeout);
}

void Connection::handleEvent(TCPServer &server, uint32_t events)
{
	(void)server;
	if (events & EPOLLHUP && events & EPOLLRDHUP)
		this->setDeletable();
	if (this->state == CLOSING && this->write_buffer.size() == 0
		&& this->read_buffer.size() == 0)
		this->setDeletable();
	if (events & EPOLLIN && this->state == CONNECTED)
	{
		TCPServer::AlarmManager.reschedule(this->alarmTimeout, this->engineConfig.max_timeout);
		this->handleRead();
		server.getHandler().onDataReceived(*this);
	}
	if (events & EPOLLOUT && this->state != DELETABLE)
	{
		TCPServer::AlarmManager.reschedule(this->alarmTimeout, this->engineConfig.closing_timeout);
		if (this->actual_job != NULL)
			this->actual_job->execute();
		this->handleWrite();
	}
}

void Connection::addJob(IJob *job)
{
	if (this->actual_job == NULL)
	{
		this->setDeletable();
		return ;
	}
	this->actual_job = job;
	this->setWritable(true);
}

void Connection::setWritable(bool writable)
{
	if (writable)
		this->server.setPollEvent(*this, CONNECTION_EVENTS | EPOLLOUT);
	else
		this->server.setPollEvent(*this, CONNECTION_EVENTS);
}

void Connection::timeout(Alarm<Connection *> &alarm)
{
	(void)alarm;
	this->setDeletable();
}

void Connection::setDeletable(void)
{
	this->state = DELETABLE;
	this->setWritable(false);
	this->server.dropConnection(this);
}

void Connection::handleRead(void)
{
	if (this->read_buffer.size() > this->engineConfig.max_read_limit)
	{
		this->setDeletable();
		return ;
	}

	uint8_t	buffer[4096];
	ssize_t n = this->client_socket.receive(buffer, sizeof(buffer));
	if (n > 0)
		this->read_buffer.insert(this->read_buffer.end(), buffer, buffer + n);
	else if (n == 0)
	{
		TCPServer::AlarmManager.reschedule(this->alarmTimeout, this->engineConfig.closing_timeout);
		this->state = CLOSING;
	}
}

void Connection::handleWrite(void)
{
	const uint8_t	*buffer_ptr = &this->write_buffer[this->bytes_sended];
	ssize_t			remaining;
	ssize_t			sent;

	if (this->bytes_sended >= this->write_buffer.size())
		this->clearWriteBuffer();
	remaining = this->write_buffer.size() - this->bytes_sended;
	sent = this->client_socket.send(buffer_ptr, remaining);
	if (sent > 0)
		this->bytes_sended += sent;
	if (this->bytes_sended >= this->write_buffer.size())
		this->clearWriteBuffer();
}

void Connection::sendData(const uint8_t *data, size_t len)
{
	this->setWritable(true);
	this->write_buffer.insert(this->write_buffer.end(), data, data + len);
}

void Connection::sendData(const std::string &data)
{
	this->setWritable(true);
	this->write_buffer.insert(this->write_buffer.end(), data.begin(),
		data.end());
}

void Connection::consumeReadData(size_t n)
{
	this->bytes_received += n;
	if (this->bytes_received >= this->read_buffer.size())
		this->clearReadBuffer();
}

const uint8_t *Connection::getReadBufferPtr() const
{
	return (&this->read_buffer[this->bytes_received]);
}

const std::vector<uint8_t>& Connection::getReadVector() const
{
	return (this->read_buffer);
}

size_t Connection::getReadBufferSize() const
{
	return (this->read_buffer.size() - this->bytes_received);
}

void Connection::clearReadBuffer()
{
	this->read_buffer.clear();
	this->bytes_received = 0;
}

void Connection::clearWriteBuffer()
{
	this->setWritable(false);
	this->write_buffer.clear();
	this->bytes_sended = 0;
}

void Connection::compactReadBuffer()
{
	this->clearReadBuffer();
	this->read_buffer.resize(4096);
}

void Connection::compactWriteBuffer()
{
	this->clearWriteBuffer();
	this->write_buffer.resize(WRITE_BUFFER_DEFAULT_SIZE);
}

size_t Connection::getClientID(void) const
{
	return (this->id);
}

Socket &Connection::getSocket(void)
{
	return (this->client_socket);
}

const Socket &Connection::getSocket(void) const
{
	return (this->client_socket);
}

Connection::State Connection::getState(void) const
{
	return (this->state);
}

const Address &Connection::getAddress(void) const
{
	return (this->client_socket.getAddress());
}

const char *Connection::getStateString(State state)
{
	switch (state)
	{
#define X(el) \
	case el:  \
		return (#el);
		_CONNECTION_STATES
	default:
		return ("Unknown");
#undef X
	}
}

void	timeoutCallback(Alarm<Connection *> &alarm, Connection *connection)
{
	connection->timeout(alarm);
}

bool operator==(const Connection &lhs, const Connection &rhs)
{
	return (lhs.id == rhs.id);
}
