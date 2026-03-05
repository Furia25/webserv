/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 20:19:58 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/05 21:30:00 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _SOCKET_H
# define _SOCKET_H

# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netdb.h>

class Socket
{
public:
	Socket();
	~Socket();

	void	listen(void);
	void	bind(void);
	int		getFD(void)	const;
protected:
private:
	const int	socket_fd;
	
};

#endif // _SOCKET_H