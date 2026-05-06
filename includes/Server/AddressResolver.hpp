/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AddressResolver.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/06 02:49:54 by vdurand           #+#    #+#             */
/*   Updated: 2026/05/06 02:51:18 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _ADDRESSRESOLVER_H
# define _ADDRESSRESOLVER_H

# include <vector>
# include <stdexcept>
# include <stdint.h>
# include <cerrno>
# include <sstream>

# include "Address.hpp"

namespace AddressResolver
{
	std::vector<Address>	resolve(const std::string& host = "", const std::string& service = "80", int type = SOCK_STREAM);
	std::vector<Address>	resolve(const char *host = NULL, const char *service = "80", int type = SOCK_STREAM);
	port_t					getPortFromService(const std::string& service_name);
};

#endif // _ADDRESSRESOLVER_H