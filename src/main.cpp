/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 14:24:55 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/12 19:04:44 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <vector>
#include "Server/Address.hpp"
#include "Server/AddressResolver.hpp"
#include "Server/TCPServer.hpp"
#include "HeaderParam.hpp"
#include "Logger.hpp"

int main()
{
	TCPServer	server;

	LogConfig::setDefaultStream(std::cout);
	server.openListener("localhost", "8080");
	try {
		server.run();
	} catch (const std::exception& e)
	{
		std::cerr << "Unable to recover from fatal error : \"" << e.what() << "\"\n	errno: " << strerror(errno) << std::endl;
	}
}
