/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Itoa.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/30 13:31:23 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/06 02:40:24 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _ITOA_H
# define _ITOA_H

# include <sstream>

template <typename T>
static inline std::string itoa(T n)
{
	std::stringstream ss;
	ss << n;
	return ss.str();
}

#endif // _ITOA_H