/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TestRequestHandler.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/10 17:08:53 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/13 18:08:03 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _TESTREQUESTHANDLER_H
# define _TESTREQUESTHANDLER_H

#include "IRequestHandler.hpp"

class TestRequestHandler : public IRequestHandler
{
public:
	TestRequestHandler();
	~TestRequestHandler();
	void	onData(Connection& connection);
	void	onConnection(Connection& connection);
	void	onDisconnection(Connection& connection);
	void	onError(Connection& connection);
protected:
private:
};

#endif // _TESTREQUESTHANDLER_H