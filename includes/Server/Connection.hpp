/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 18:13:47 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/24 16:32:29 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _CONNECTION_H
# define _CONNECTION_H

# define WRITE_BUFFER_DEFAULT_SIZE	2096

# include <vector>
# include <queue>
# include <stddef.h>
# include <cstring>

# include "Config/Config.hpp"
# include "Socket.hpp"
# include "IEpollHandler.hpp"
# include "Utils/HashedTimingWheel.hpp"

# define _CONNECTION_STATES \
	X(ERRORED)\
	X(CONNECTED)\
	X(CLOSING)\
	X(DELETABLE)

class IJob;

class Connection : public IEpollHandler
{
public:
	enum State
	{
		#define X(el, ...) el,
		_CONNECTION_STATES
		#undef X
	};

	Connection(TCPServer& server, Socket& server_socket);
	virtual ~Connection();

	void			sendData(const uint8_t *data, size_t len);
	void			sendData(const std::string& data);
	void			setDeletable(void);

	void			clearReadBuffer();
	void			clearWriteBuffer();
	void			compactReadBuffer();
	void			compactWriteBuffer();

	void			addJob(IJob* job);
	size_t			getClientID(void)	const;
	Socket&			getSocket(void);
	const Socket&	getSocket(void) const;
	State			getState(void) const;
	const Address&	getAddress(void) const;

	void						consumeReadData(size_t n);
	const uint8_t				*getReadBufferPtr() const;
	const std::vector<uint8_t>&	getReadVector() const;
	size_t						getReadBufferSize() const;

	static const char		*getStateString(State state);

	friend class			TCPServer;
	friend bool				operator==(const Connection& lhs, const Connection& rhs);
	friend std::ostream&	operator<<(std::ostream& os, const Connection& connection);
protected:
private:
	TCPServer&				server;
	Socket					client_socket;
	size_t					id;

	std::vector<uint8_t>	write_buffer;
	size_t					bytes_sended;
	std::vector<uint8_t>	read_buffer;
	size_t					bytes_received;

	static size_t			last_id;
	State					state;
	Alarm<Connection *>		alarmTimeout;

	const Config::EngineConfig&	engineConfig;
	IJob	*actual_job;

	friend void		timeoutCallback(Alarm<Connection *>& alarm, Connection* connection);

	void			setWritable(bool writable);
	void			timeout(Alarm<Connection *>& alarm);
	void			handleRead(void);
	void			handleWrite(void);
	void			handleEvent(TCPServer &server, uint32_t events);

	Connection(const Connection& other);
	Connection&		operator=(const Connection& other);
};

void	timeoutCallback(Alarm<Connection *>& alarm, Connection* connection);

bool	operator==(const Connection& lhs, const Connection& rhs);

inline std::ostream&	operator<<(std::ostream& os, const Connection& connection)
{
	os << connection.getClientID() << ":" << '\"' << connection.client_socket.getAddress() << "\""
		<< ':' << Connection::getStateString(connection.state);
	return os;
}

#endif // _CONNECTION_H