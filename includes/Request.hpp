/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antoine <antoine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/04 19:17:15 by antoine           #+#    #+#             */
/*   Updated: 2026/03/04 20:12:16 by antoine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <map>

class Request
{
  public:
	Request();
	Request(const Request &other);
	~Request();
	Request &operator=(const Request &other);

  private:
	std::string _method; //  GET, POST, DELETE
	std::string _request_path; // /index.html /images/logo.png
	std::string _protocol; // HTTP/1.1
	std::map<std::string, std::string> _headers; // EX : "HOST" + "localhost:8080" delimited with ':' _header[host] = "localhost:8080"
    std::string _body // if content-length is in the request get the body
};