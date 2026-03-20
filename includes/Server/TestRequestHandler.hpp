/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TestRequestHandler.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/10 17:08:53 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/17 16:27:15 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _TESTREQUESTHANDLER_H
# define _TESTREQUESTHANDLER_H

#include "IRequestHandler.hpp"
#include "Request.hpp"
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
	std::map<size_t, Request> ongoingRequests;
};

#endif // _TESTREQUESTHANDLER_H