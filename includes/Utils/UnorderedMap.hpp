/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   UnorderedMap.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/21 14:18:44 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/22 18:53:48 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _UNORDEREDMAP_H
# define _UNORDEREDMAP_H

# include <stddef.h>
# include <utility>

# include "Hash.hpp"
# include "Optional.hpp"

# define HASHMAP_DEFAULT_CAPACITY	16
# define HASHMAP_REHASH_GROWTH	2.0

# define _HashMapTemplate_ template <class Key, class Value, typename HashFunctor>
# define _HashMapDef_ HashMap<Key, Value, HashFunctor>

template <class Key, class Value, typename HashFunctor = Hash<Key> >
class HashMap
{

public:	/*Public types*/
	typedef Key						key_type;
	typedef Value					mapped_type;
	typedef std::pair<Key, Value>	value_type;
	typedef HashFunctor				hash_functor;

public: /*Slot Internal Structure*/
	struct Slot
	{
		Optional<value_type>	data;
		bool					is_deleted;

		Slot() : state(EMPTY) {}
		inline bool	empty() const		{ return !this->data.has_value() && !this->is_deleted; };
		inline bool	occupied() const	{ return this->data.has_value; };
		inline bool	deleted() const		{ return this->is_deleted};
	};

public: /*Iterator*/
	class iterator
	{
		
	};
	class const_iterator
	{
		
	};

public:
	HashMap(size_t base_capacity = 16);
	HashMap(const HashMap& other);
	HashMap&			operator= (const HashMap& other);
	~HashMap();

	iterator			find(const key_type& key);
	const_iterator		find(const key_type& key) const;
	
	bool				contain(const key_type& key) const;
	
	void				insert(const key_type& key, const mapped_type& value);
	mapped_type&		at(const key_type& key);
	const mapped_type&	at(const key_type& key) const;

	iterator			begin();
	const_iterator		begin() const;
	iterator			end();
	const_iterator		end() const;

	size_t				size() const;
	bool				empty() const;
	float				load_factor() const;
	float				max_load_factor() const;	
protected:
private:
	Slot*		table;
	size_t		capacity;
	size_t		slot_used;
	float		growth_factor;
	float		_max_load_factor
	HashFunctor	hasher;

	size_t		get_hash_index(const key_type& key) const;
	size_t		probe(const key_type& key) const;
	void		_rehash(size_t new_capacity);
};

_HashMapTemplate_
HashMap<Key, Value, HashFunctor>::HashMap(size_t base_capacity):
	table(new Slot[base_capacity]),
	capacity(base_capacity),
	size(0),
	growth_factor(1.5),
	max_load_factor(0.7),
	hasher(HashFunctor()) {}

_HashMapTemplate_
inline HashMap<Key, Value, HashFunctor>::HashMap(const HashMap& other)
{
}

_HashMapTemplate_
inline HashMap<Key, Value, HashFunctor>::~HashMap()
{
	delete[] this->table;
}

_HashMapTemplate_
inline void HashMap<Key, Value, HashFunctor>::insert(const key_type &key, const mapped_type &value)
{
	if (this->load_factor() >= this->_max_load_factor)
		this->_rehash(this->capacity * this->growth_factor);
	size_t index = this->probe(key);
	if (index == SIZE_MAX)
		throw std::runtime_error("Unable to insert value into an HashMap");
	Slot& slot = this->table[index];
	if (slot.occupied())
		slot.data->second = value;
	else
	{
		slot.is_deleted = false;
		slot.data = std::make_pair(key, value);
	}
	this->slot_used++;
}

template <class Key, class Value, typename HashFunctor>
inline _HashMapDef_::mapped_type& HashMap<Key, Value, HashFunctor>::at(const key_type &key)
{
	size_t	index = this->probe(key);
	if (index == SIZE_MAX || !this->table[index].occupied())
		throw std::runtime_error("Key entry doesn't exist in hashmap");
	return (this->table[index].data->second);
}

template <class Key, class Value, typename HashFunctor>
inline const _HashMapDef_::mapped_type& HashMap<Key, Value, HashFunctor>::at(const key_type &key) const
{
		size_t	index = this->probe(key);
	if (index == SIZE_MAX || !this->table[index].occupied())
		throw std::runtime_error("Key entry doesn't exist in hashmap");
	return (this->table[index].data->second);
}

_HashMapTemplate_
inline HashMap<Key, Value, HashFunctor>::iterator HashMap<Key, Value, HashFunctor>::find(const key_type &key)
{
	size_t	index = this->probe(key);

	if (index == SIZE_MAX || !table[index].occupied())
		return this->end();
	return iterator(); // TODO
}

_HashMapTemplate_
inline HashMap<Key, Value, HashFunctor>::const_iterator HashMap<Key, Value, HashFunctor>::find(const key_type &key) const
{
	size_t	index = this->probe(key);

	if (index == SIZE_MAX || !table[index].occupied())
		return this->end();
	return const_iterator(); // TODO
}

_HashMapTemplate_
inline bool HashMap<Key, Value, HashFunctor>::contain(const key_type &key) const
{
	return this->find(key) != this->end();
}

_HashMapTemplate_
inline size_t HashMap<Key, Value, HashFunctor>::size() const
{
	return this->slot_used;
}

_HashMapTemplate_
inline bool HashMap<Key, Value, HashFunctor>::empty() const
{
	return this->slot_used == 0;
}

_HashMapTemplate_
inline float HashMap<Key, Value, HashFunctor>::load_factor() const
{
	return static_cast<float>(this->slot_used) / this->capacity;
}

_HashMapTemplate_
inline float HashMap<Key, Value, HashFunctor>::max_load_factor() const
{
	return this->_max_load_factor;
}

_HashMapTemplate_
inline size_t HashMap<Key, Value, HashFunctor>::get_hash_index(const key_type &key) const
{
	return this->hasher(key) % this->capacity;
}

_HashMapTemplate_
inline size_t HashMap<Key, Value, HashFunctor>::probe(const key_type &key) const
{
	size_t	index = this->get_hash_index(key);
	size_t	first_tombstone = SIZE_MAX;
	size_t	i = 0;

	while (i < this->capacity)
	{
		Slot& slot = this->table[index];
		if (slot.occupied() && slot.data->first == key)
			return index;
		if (slot.empty())
			break ;
		if (slot.deleted() && first_tombstone == SIZE_MAX)
			first_tombstone = index;
		index = (index + 1) % _capacity;
		i++;
	}
	return first_tombstone;
}

# undef _HashMapDef_
# undef _HashMapTemplate_

#endif // _UNORDEREDMAP_H
