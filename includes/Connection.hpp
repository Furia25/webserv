/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 18:13:47 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/08 16:15:51 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _CONNECTION_H
# define _CONNECTION_H

# include <vector>
# include <stddef.h>

# include "Socket.hpp"

class Connection
{
public:
	Connection();
	~Connection();

	

	size_t			getClientID(void)	const;
	Socket&			getSocket(void);
	const Socket&	getSocket(void) const;

	friend bool		operator==(const Connection& lhs, const Connection& rhs);
protected:
private:
	Socket					client_socket;
	size_t					id;

	std::vector<uint8_t>	write_buffer;
	size_t					bytes_sended;

	std::vector<uint8_t>	read_buffer;
	size_t					bytes_received;

	static size_t			last_id;

	Connection(const Connection& other);
	Connection&		operator=(const Connection& other);
};

bool	operator==(const Connection& lhs, const Connection& rhs);

#endif // _CONNECTION_H