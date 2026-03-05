/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Address.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 20:30:25 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/05 21:30:42 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _ADDRESS_H
# define _ADDRESS_H

# include <unistd.h>
# include <netdb.h>
# include <cstring>
# include <stdexcept>
# include <cerrno>
# include <sstream>
# include <iostream>

using SocketAddr = struct sockaddr_storage;

class Address
{
public:
	Address(const std::string &host = "", int service = 8080);
	Address(const char *host = NULL, const char *service = "8080");
	Address(const Address& other);
	~Address();
	Address& operator=(const Address& other);

	std::string				toString(bool host = true, bool service = true) const;

	bool					isIPv6(void) const;

	const std::string&		getHost(void) const;
	const int				getService(void) const;
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
	int			service;

	void		init_address(const char *host, const char *service);
};

std::ostream&	operator<<(std::ostream& os, const Address& addr);

#endif // _ADDRESS_H