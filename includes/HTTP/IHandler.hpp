/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IHandler.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 18:39:26 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/21 14:11:15 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


# ifndef _IHANDLER_H
# define _IHANDLER_H
# include "Request.hpp"
# include "../Config/Config.hpp"
# include "../Server/Connection.hpp"
# include "Response.hpp"
# include "IJob.hpp"

class IHandler : public IJob
{
public:
    virtual ~IHandler() {};
};

#endif // _IHANDLER_H