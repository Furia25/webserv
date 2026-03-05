/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TCPServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 17:43:15 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/05 20:14:18 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _TCPSERVER_H
# define _TCPSERVER_H

# include <vector>
# include <cstring>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netdb.h>
# include <stdexcept>
# include <cerrno>
# include <sstream>

# include "IRequestHandler.hpp"

# define MAX_PENDING_CONNECTION	10

class TCPServer
{
public:
	TCPServer();
	~TCPServer();

	void	openListener(const std::string& ip, unsigned int port);
	void	openListener(const char *ip, const char *service);
	void	bindHandler(IRequestHandler& handler);
protected:
private:
	const int				port;
	std::vector<int>		sockets;
	IRequestHandler&		handler;
	std::vector<Connection>	connections;
};

#endif // _TCPSERVER_H