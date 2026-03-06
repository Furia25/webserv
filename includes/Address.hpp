/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Address.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 20:30:25 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/06 03:18:28 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _ADDRESS_H
# define _ADDRESS_H

# include <unistd.h>
# include <netdb.h>
# include <cstring>
# include <iostream>
# include <sstream>

typedef struct sockaddr_storage	SocketAddr;

# define MAX_PORT	UINT16_MAX

class Address
{
public:
	Address(const std::string& host, const std::string& service, const struct addrinfo *raw_addr);
	Address(const Address& other);
	~Address();
	Address& operator=(const Address& other);

	std::string				toString(bool host = true, bool service = true) const;

	bool					isIPv6(void) const;

	const std::string&		getHost(void) const;
	const std::string&		getService(void) const;
	const struct sockaddr*	getSockAddr(void) const;
	socklen_t				getAddrLen(void) const;
	int						getFamily(void) const;
	int						getType(void) const;
	int						getProtocol(void) const;
	int						getFlags(void) const;
protected:
private:
	SocketAddr	*data;
	socklen_t	addr_len;
	int			family;
	int			type;
	int			protocol;
	int			flags;

	std::string	host;
	std::string	service;

	void		init_address(const char *host, const char *service, int type = SOCK_STREAM);
};

std::ostream&	operator<<(std::ostream& os, const Address& addr);

#endif // _ADDRESS_H