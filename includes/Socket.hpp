/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 20:19:58 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/06 04:27:49 by vdurand          ###   ########.fr       */
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

class Socket
{
public:
	Socket();
	~Socket();

	void	open(int type, int protocol, int domain);
	void	open_dual(int type, int protocol);

	void	setReuseAddr(bool enable);
	void	setNoDelay(bool enable);
	void	setDualStack(bool enable);
	enum State { CLOSED, OPEN, BOUND, LISTENING, CONNECTED };

	State	getState() const;
protected:
private:
	int			socket_fd;
	State		state;

	template <typename T>
	void	setOption(int level, int option, const T& value);
};

class SocketException : public std::runtime_error
{
public:
	SocketException(const std::string& msg, int code = 0)
		: std::runtime_error(msg), error_code(code) {}
	int getErrorCode() const { return error_code; }
private:
	int error_code;
};

#endif // _SOCKET_H

template <typename T>
inline void Socket::setOption(int level, int option, const T& value)
{
	if (setsockopt(socket_fd, level, option, &value, sizeof(value)) < 0)
		throw SocketException("Couldn't configure socket option");
}
