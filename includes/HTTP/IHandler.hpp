/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IHandler.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 18:39:26 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/27 14:26:01 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


# ifndef _IHANDLER_H
# define _IHANDLER_H

# include "Request.hpp"
# include "Config/Config.hpp"
# include "Server/Connection.hpp"
# include "Response.hpp"
# include "IJob.hpp"

class IHandler : public IJob
{
public:
	virtual ~IHandler() {};
};

#endif // _IHANDLER_H