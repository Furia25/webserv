/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AutoindexHandler.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 10:45:39 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/27 14:25:19 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _AUTOINDEXHANDLER_H
# define _AUTOINDEXHANDLER_H

# include "HTTP/IHandler.hpp"
# include <dirent.h>

class AutoindexHandler : public IHandler
{
private:
	const Request		request;
	Connection&			connection;
	std::string			physical_path;
	bool				isFinished;

public:
	AutoindexHandler(const Request req, Connection &conn, const std::string& path);

	bool execute();

};

#endif // _AUTOINDEXHANDLER_H