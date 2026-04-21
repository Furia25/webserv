/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IJob.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/20 18:00:00 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/21 14:22:24 by antbonin         ###   ########.fr       */
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
