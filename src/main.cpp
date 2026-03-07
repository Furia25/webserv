/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 14:24:55 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/07 17:51:43 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "MIME.hpp"
#include <iostream>
#include <vector>
#include "Address.hpp"
#include "AddressResolver.hpp"
#include "Socket.hpp"

int main()
{
	std::vector<Address> addresses = AddressResolver::resolve("google.com", "8080");
	std::cout << addresses[0] << std::endl;
	Socket	sock;
	sock.open(SOCK_STREAM, AF_INET, IPPROTO_TCP);
	sock.connect(addresses);
	std::string request = 
		"GET / HTTP/1.1\r\n"
		"Host: www.google.com\r\n"
		"Connection: close\r\n" // On demande à Google de fermer après la réponse
		"\r\n";
	
	std::cout << sock << std::endl;
}
