/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Debugging.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/13 17:09:12 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/13 17:09:44 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _DEBUGGING_H
# define _DEBUGGING_H

# include <iostream>
# include <vector>

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& container)
{
	typename std::vector<T>::const_iterator it = container.begin();
	for (; it != container.end(); it++)
	{
		os << *it << " ";
	}
	return os;
}

#endif // _DEBUGGING_H