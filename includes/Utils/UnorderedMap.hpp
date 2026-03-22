/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UnorderedMap.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/21 14:18:44 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/22 15:51:39 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _UNORDEREDMAP_H
# define _UNORDEREDMAP_H

# include <stddef.h>
# include <utility>

template <typename Key>
struct Hash
{
	size_t	operator()(const Key& key) const;
};

template <class Key, class T, typename hashResolver = Hash<Key>>
class UnorderedMap
{
public:
	enum State {EMPTY, OCCUPIED, DELETED};
	struct Slot
	{
		std::pair<Key, T>	value;
		State				state;

		Slot() : state(EMPTY) {}
	}
protected:
private:

};

#endif // _UNORDEREDMAP_H