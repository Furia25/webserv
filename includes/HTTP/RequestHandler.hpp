/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   RequestHandler.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/10 17:08:53 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/20 14:00:48 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# ifndef _RequestHandler_H
# define _RequestHandler_H

# include "../Server/IRequestHandler.hpp"
# include "Config/Config.hpp"
# include "HTTP/RequestBuilder.hpp"
# include "map"
# include "HTTP/Router.hpp"

class RequestHandler : public IRequestHandler
{
public:
	RequestHandler(const Config::AppConfig& config);
	~RequestHandler();
	void		onDataReceived(Connection& connection);
	void		onConnection(Connection& connection);
	void		onDisconnection(Connection& connection);
	void		onError(Connection& connection);
protected:
private:
	void		onDisconnection(Connection &connection);
	void		manageJobs(Connection &connection);
	void		dispatchError(int id, Connection& connection, HTTPCode code, const Config::ServerConfig* host, const Request* req = NULL);
	void		handleStaticRoute(int id, Connection& connection, const Request& final_request, const Config::RouteConfig* route, const Config::ServerConfig* host, std::string physical_path);
	Router router;
	std::map<size_t, RequestBuilder>			ongoingRequests;
	HashMap<int, IJob*>							ongoingJobs;
};

#endif // _RequestHandler_H