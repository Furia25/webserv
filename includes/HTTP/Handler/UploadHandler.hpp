/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UploadHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 17:08:05 by vdurand           #+#    #+#             */
/*   Updated: 2026/06/08 20:53:31 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _UPLOADHANDLER_H
# define _UPLOADHANDLER_H

# include "Config/Config.hpp"
# include "HTTP/AHandler.hpp"
# include "HTTP/Response.hpp"

class UploadHandler : public AHandler
{
public:
	UploadHandler(
		HTTPHandler& handler,
		Connection& connection,
		const Request& request,
		Body& body,
		const Router::RouteResult& route_result,
		HTTPCode status_code = HTTPCode::OK)
	: AHandler(handler, connection, request, body, route_result, status_code), 
		uploadConfig(static_cast<const Config::UploadConfig&>(*route_result.route)), copyState(INIT) {};

	void	onExecute();
	void	onCreation();
private:
	enum CopyState
	{
		INIT,
		COPYING,
	};
	CopyState	copyState;
	std::ifstream	srcFile;
	std::ofstream	dstFile;
	std::string		tempPath;
	const Config::UploadConfig&	uploadConfig;
};

#endif // _UPLOADHANDLER_H