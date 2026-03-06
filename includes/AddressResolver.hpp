/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AddressResolver.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/06 02:49:54 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/06 03:18:47 by vdurand          ###   ########.fr       */
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

class AddressResolver
{
public:
	static std::vector<Address>	resolve(const std::string& host = "", const std::string& service = "80", int type = SOCK_STREAM);
	static std::vector<Address> resolve(const char *host = NULL, const char *service = "80", int type = SOCK_STREAM);
protected:
private:
	AddressResolver();
	~AddressResolver();
};

#endif // _ADDRESSRESOLVER_H