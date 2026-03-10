/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 20:19:58 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/10 18:43:49 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _SOCKET_H
# define _SOCKET_H

# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <netdb.h>
# include <cstring>
# include <stdexcept>
# include <vector>
# include <iostream>
# include <cerrno>
# include <fcntl.h>

# include "Address.hpp"

# define SOCKET_DEBUG	false
# if SOCKET_DEBUG == true
	# define SOCKET_DEBUG_ERROR	do {std::cerr << this->toString((int)errno) << "\n";} while (0)
#else
	# define SOCKET_DEBUG_ERROR	
#endif

# define ESOCK_INVALID	"Invalid socket, not opened."

class Socket
{
public:
	Socket();
	~Socket();

	void		open(int type, int domain, int protocol = 0);
	void		open_dual(int type, int protocol = 0);

	void		bind(const Address& address);
	void		bind(const std::vector<Address>&	addresses);

	void		connect(const Address& address);
	void		connect(const std::vector<Address>&	addresses);

	void		listen(unsigned int backlog);

	void		accept(Socket& server);

	ssize_t		receive(void *buffer, size_t size, int flags = 0);
	ssize_t		send(const void *buffer, size_t size, int flags = MSG_NOSIGNAL);

	void		setReuseAddr(bool enable);
	void		setNoDelay(bool enable);
	void		setDualStack(bool enable);
	void		setIOBlocking(bool blocking);

	int			fcntl(int command, long arg);

	std::string	toString(int error = 0) const;

	enum State { CLOSED, OPEN, BOUND, LISTENING, CONNECTED };

	State			getState() const;
	const Address&	getAddress() const;
	int				getFd() const;
protected:
private:
	int			domain;
	int			socket_fd;
	int			type;
	int			protocol;
	Address		address;
	State		state;

	Socket(const Socket& other);
	Socket&		operator=(const Socket& other);
	const char* stateToString() const;

	template <typename T>
	void	setOption(int level, int option, const T& value);
};

std::ostream&	operator<<(std::ostream& os, const Socket& socket);

class SocketException : public std::runtime_error
{
public:
	SocketException(const std::string& msg, int code = 0)
		: std::runtime_error(msg), error_code(code) {}
	SocketException(const std::string& type, const std::string& msg, int code = 0)
		: std::runtime_error(type + ": " + msg), error_code(code) {}
	int getErrorCode() const { return error_code; }
private:
	int error_code;
};

template <typename T>
inline void Socket::setOption(int level, int option, const T &value)
{
	if (setsockopt(socket_fd, level, option, &value, sizeof(value)) == -1)
		throw SocketException("Couldn't configure socket option");
}

#endif // _SOCKET_H

