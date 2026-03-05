/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Address.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 20:30:25 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/05 20:58:55 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _ADDRESS_H
# define _ADDRESS_H

# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netdb.h>
# include <cstring>
# include <stdexcept>
# include <cerrno>
# include <sstream>

using SocketAddr = struct sockaddr_storage;

class Address
{
public:
	Address(const std::string& host = "", int service = 8080);
	Address(const char *host = NULL, const char *service = "8080");
	Address(const Address& other);
	~Address();
	Address& operator=(const Address& other);

	const std::string&	getNode(void) const;
	const int			getService(void) const;
protected:
private:
	SocketAddr	*data;
	socklen_t	addr_len;
	int			family;
	int			type;
	int			protocol;
	int			flags;
	std::string	host;
	
	int			service;
};

#endif // _ADDRESS_H