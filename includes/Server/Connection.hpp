/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 18:13:47 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/13 18:20:18 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _CONNECTION_H
# define _CONNECTION_H

# include <vector>
# include <stddef.h>
# include <cstring>

# include "Socket.hpp"
# include "IEpollHandler.hpp"

# define READ_SIZE	4096
# define READ_LIMIT	16384

# define READ_BUFFER_DEFAULT_SIZE	4096
# define WRITE_BUFFER_DEFAULT_SIZE	2096

# define _CONNECTION_STATES \
	X(ERRORED)\
	X(CONNECTED)\
	X(CLOSING)\
	X(DELETABLE)

class Connection : public IEpollHandler
{
public:
	enum State
	{
		#define X(el, ...) el,
		_CONNECTION_STATES
		#undef X
	};

	Connection(Socket& server_socket);
	virtual ~Connection();

	void			handleEvent(TCPServer &server, uint32_t events);
	void			handleRead(void);
	void			handleWrite(void);

	void			sendData(const uint8_t *data, size_t len);
	void			sendData(const std::string& data);

	void			consumeReadData(size_t n);
	const uint8_t	*getReadBufferPtr() const;
	size_t			getReadBufferSize() const;

	void			clearReadBuffer();
	void			clearWriteBuffer();
	void			compactReadBuffer();
	void			compactWriteBuffer();

	size_t			getClientID(void)	const;
	Socket&			getSocket(void);
	const Socket&	getSocket(void) const;
	State			getState(void) const;
	const Address&	getAddress(void) const;

	static const char		*getStateString(State state);

	friend bool				operator==(const Connection& lhs, const Connection& rhs);
	friend std::ostream&	operator<<(std::ostream& os, const Connection& connection);
protected:
private:
	Socket					client_socket;
	size_t					id;

	std::vector<uint8_t>	write_buffer;
	size_t					bytes_sended;

	std::vector<uint8_t>	read_buffer;
	size_t					bytes_received;

	static size_t			last_id;
	State					state;

	Connection(const Connection& other);
	Connection&		operator=(const Connection& other);
};

bool	operator==(const Connection& lhs, const Connection& rhs);

inline std::ostream&	operator<<(std::ostream& os, const Connection& connection)
{
	os << connection.getClientID() << ":" << '\"' << connection.client_socket.getAddress() << "\""
		<< ':' << Connection::getStateString(connection.state);
	return os;
}

#endif // _CONNECTION_H