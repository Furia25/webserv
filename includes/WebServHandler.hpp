/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antoine <antoine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/06 19:05:32 by antoine           #+#    #+#             */
/*   Updated: 2026/03/06 19:06:37 by antoine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _WEBSERVHANDLER_H
# define _WEBSERVHANDLER_H

# include "IRequestHandler.hpp"
#include "Request.hpp"
#include <map>

class WebServHandler : public IRequestHandler
{
  public:
	WebServHandler();
	WebServHandler(const WebServHandler &other);
	~WebServHandler();
	WebServHandler &operator=(const WebServHandler &other);

  protected:
  private:
	std::map<size_t, Request> _ongoingRequests;
};

#endif // _WEBSERVHANDLER_H