/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DeleteHandler.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 12:44:02 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/27 14:25:11 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _DELETEHANDLER_H
# define _DELETEHANDLER_H

# include "HTTP/IHandler.hpp"
# include "HTTP/Response.hpp"
# include "Utils/FileSystem.hpp"
# include <cstdio>

class DeleteHandler : public IHandler
{
private:
	Connection &connection;
	std::string physical_path;
	bool isFinished;

public:
	DeleteHandler(Connection &conn, const std::string &path);

	bool execute();
};

# endif // _DELETEHANDLER_H
