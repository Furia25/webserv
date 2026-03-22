/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Hash.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/22 17:33:03 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/22 17:33:36 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _HASH_H
# define _HASH_H

/*Default Functor Template*/
template <typename Key>
struct Hash
{
	size_t	operator()(const Key& key) const;
};

#endif // _HASH_H