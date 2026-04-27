/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IJob.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 18:00:00 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/27 14:25:53 by vdurand          ###   ########.fr       */
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
