/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antoine <antoine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 14:07:16 by antoine           #+#    #+#             */
/*   Updated: 2026/03/04 19:19:26 by antoine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#define PORT 8080

int	main(int ac, char **av)
{
	int server_fd; // the file descriptor of the server
	int new_socket; // the file descriptor - request from the client + appropriate response in HTPP
	long valread; // len of the request
	struct sockaddr_in address; // 
	int addrlen;
	int opt = 1;

	addrlen = sizeof(address);

	// start of the socket in ipv4 = AF_INET SOCK_STREAM = TCP 
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("in socket");
		exit(EXIT_FAILURE);
	}
	// set the option of the socket to reuse address after the end of the process
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
	{
		perror("in socket option");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_port = htons(PORT); // get the port in endian 100000 instead of 000001
	address.sin_addr.s_addr = INADDR_ANY; // listen to every interface such as ethernet wifi , bluetooth 
	memset(address.sin_zero, '\0', sizeof(address.sin_zero));
	// we bind the socket to a port with the struct address 
	if (bind(server_fd, reinterpret_cast<struct sockaddr *>(&address),
			sizeof(address)) < 0)
	{
		perror("in binding");
		exit(EXIT_FAILURE);
	}
	// open the socket
	if (listen(server_fd, 10) < 0)
	{
		perror("in listen");
		exit(EXIT_FAILURE);
	}

	std::cout << "Server waiting on port " << PORT << " ..." << std::endl;

	while (true)
	{
		// get request from client
		if ((new_socket = accept(server_fd, reinterpret_cast<struct sockaddr *>(&address), reinterpret_cast<socklen_t *>(&addrlen))) < 0)
		{
			continue;
		}
		// read to the buffer the client request , stocked in temp
		std::string full_request = "";
		char temp_buffer[1024];
		size_t bytes_received;
		// reading the request block per block filling the string "full_request"
		while(true)
		{
			bytes_received = recv(new_socket, temp_buffer, sizeof(temp_buffer), 0);
			if (bytes_received > 0)
			{
				full_request.append(temp_buffer, bytes_received);
				if (full_request.find("\r\n\r\n") != std::string::npos)
					break;
			}
			else if (bytes_received == 0)
				break;
			else 		
			{
				perror("recv error");
				break;
			}
		}
		
		// send a proper htpp response to print "hello world"
		std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\nConnection: close\r\n\r\nHello, World!";
		// use send with MSG_NOSIGNAL to prevent sigpipe error
		send(new_socket, response.c_str(), response.size(), MSG_NOSIGNAL);
		close(new_socket);
	}
}