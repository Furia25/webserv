/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IJob.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 17:50:56 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/20 17:55:43 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _IJOB_H
# define _IJOB_H

# include "Connection.hpp"

class IJob
{
public:
	virtual void	jobContinue(Connection& connection) = 0;
};

#endif // _IJOB_H