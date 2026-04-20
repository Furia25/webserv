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

class RequestHandler : public IRequestHandler
{
public:
	RequestHandler(const Config& config);
	~RequestHandler();
	void	onDataReceived(Connection& connection);
	void	onConnection(Connection& connection);
	void	onDisconnection(Connection& connection);
	void	onError(Connection& connection);
protected:
private:
	std::map<size_t, RequestBuilder> ongoingRequests;
	const Config& serverConfig;
};

#endif // _RequestHandler_H