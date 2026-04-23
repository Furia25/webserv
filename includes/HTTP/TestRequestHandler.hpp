/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TestRequestHandler.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/10 17:08:53 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/23 03:00:34 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _TESTREQUESTHANDLER_H
# define _TESTREQUESTHANDLER_H

#include "Server/IRequestHandler.hpp"
#include "HTTP/Request.hpp"
#include "map"

class TestRequestHandler : public IRequestHandler
{
public:
	TestRequestHandler();
	~TestRequestHandler();
	void	onDataReceived(Connection& connection);
	void	onConnection(Connection& connection);
	void	onDisconnection(Connection& connection);
	void	onError(Connection& connection);
protected:
private:
	std::map<int, Request> ongoingRequests;
};

#endif // _TESTREQUESTHANDLER_H