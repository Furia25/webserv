/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 18:13:47 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/05 19:27:50 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _CONNECTION_H
# define _CONNECTION_H

# include <span>
# include "stddef.h"

class Connection
{
public:
	Connection();
	Connection(const Connection& other);
	~Connection();
	Connection& operator=(const Connection& other);

	size_t	getClientID(void)	const;
protected:
private:
	int		client_fd;
	size_t	client_id;
};

#endif // _CONNECTION_H