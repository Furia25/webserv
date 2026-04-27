/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IJob.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 18:34:43 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/27 16:18:19 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef IJOB_HPP
# define IJOB_HPP

class IJob
{
public:
	virtual ~IJob() {}

	virtual bool execute() = 0;
};

#endif // IJOB_HPP
