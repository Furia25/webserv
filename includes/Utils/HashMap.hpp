/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HashMap.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/23 00:38:53 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/23 19:10:19 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _HASHMAP_H
# define _HASHMAP_H

# include <cstring>
# include <stddef.h>
# include <utility>

# include "Hash.hpp"
# include "Optional.hpp"

# define HASHMAP_DEFAULT_CAPACITY	16
# define HASHMAP_REHASH_GROWTH	2.0
# define HASHMAP_DEFAULT_MAX_LOAD_FACTOR 0.65f

# define _HashMapTemplate_ template <class Key, class Value, typename HashFunctor>
# define _HashMapDef_ HashMap<Key, Value, HashFunctor>
# define _HashMapType_(T)  typename _HashMapDef_::T

namespace _HashMap_
{
	typedef uint8_t SlotMeta;
	struct Meta
	{
		static const unsigned char EMPTY   = 0xFF;
		static const unsigned char DELETED = 0xFE;

		static bool is_empty(unsigned char m)		{ return m == EMPTY; }
		static bool is_deleted(unsigned char m)		{ return m == DELETED; }
		static bool is_occupied(unsigned char m)	{ return m < 0x80; }
		static unsigned char h7(size_t hash)		{ return static_cast<unsigned char>(hash & 0x7F); }

	private:
		Meta();
	};
}

template <class Key, class Value, typename HashFunctor = Hash<Key> >
class HashMap
{

public:	/*Public types*/
	typedef Key						key_type;
	typedef Value					mapped_type;
	typedef std::pair<Key, Value>	value_type;
	typedef HashFunctor				hash_functor;

	typedef ptrdiff_t				difference_type;
	typedef size_t					size_type;

	typedef RawStorage<value_type>	Slot;

private:

public: /*Iterator*/
	# define _HASHMAP_ITERATOR_(iterator_type) \
	class iterator_type : public std::iterator<std::forward_iterator_tag, \
		value_type, \
		ptrdiff_t, \
		value_type*, \
		value_type&> \
	{ \
	private: \
		HashMap*	map; \
		size_t		index; \
		void	skip_empty() \
		{ \
			while (this->index < this->map->capacity \
					&& !_HashMap_::Meta::is_occupied(this->map->meta[this->index])) \
				this->index++; \
		} \
	public: \
		iterator_type(HashMap* map, size_t index) : map(map), index(index) { this->skip_empty(); }; \
		value_type&		operator* () const	{ return *this->map->table[this->index].ptr(); }; \
		value_type		*operator->() const	{ return this->map->table[this->index].operator->(); }; \
		bool			operator==(const iterator_type& other) const { return index == other.index; } \
		bool			operator!=(const iterator_type& other) const { return index != other.index; } \
		\
		iterator_type&	operator++() \
		{ \
			this->index++; \
			this->skip_empty(); \
			return *this; \
		} \
		\
		iterator_type	operator++(int) \
		{ \
			iterator_type temp = *this; \
			++(*this); \
			return temp; \
		} \
	}; \
	friend class iterator_type

	_HASHMAP_ITERATOR_(iterator);
	_HASHMAP_ITERATOR_(const_iterator);
public:
	HashMap(
		size_t base_capacity = 0,
		float max_load_factor = HASHMAP_DEFAULT_MAX_LOAD_FACTOR,
		float growth_factor = HASHMAP_REHASH_GROWTH
	);
	HashMap(const HashMap& other);
	HashMap&			operator= (const HashMap& other);
	~HashMap();

	iterator			find(const key_type& key);
	const_iterator		find(const key_type& key) const;
	
	bool				contain(const key_type& key) const;

	void				insert(const key_type& key, const mapped_type& value);
	mapped_type&		at(const key_type& key);
	const mapped_type&	at(const key_type& key) const;

	iterator			begin()			{ return iterator(this, 0); };
	const_iterator		begin() const	{ return const_iterator(this, 0); };
	iterator			end()			{ return iterator(this, this->capacity); };
	const_iterator		end() const		{ return const_iterator(this, this->capacity); };

	bool				erase(const key_type& key);
	void				clear(void);
	void				rehash(size_t new_capacity);

	size_t				size() const;
	bool				empty() const;
	float				load_factor() const;
	float				max_load_factor() const;
private:
	Slot					*table;
	_HashMap_::SlotMeta		*meta;
	size_t					capacity;
	size_t					slot_used;
	float					growth_factor;
	float					_max_load_factor;
	size_t					tombstone_count;
	HashFunctor				hasher;

	void		insert_unsafe(const key_type& key, const mapped_type& value);
	size_t		probe(const key_type& key) const;
	size_t		probe(size_t h, unsigned char h7, const key_type& key) const;
	void		_rehash(size_t new_capacity);
};

inline size_t next_pow2(size_t n)
{
	if (n == 0) return HASHMAP_DEFAULT_CAPACITY;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	n |= n >> 16;
	n |= n >> 32;
	return n + 1;
}

_HashMapTemplate_
_HashMapDef_::HashMap(size_t base_capacity, float max_load_factor, float growth_factor) :
	table(base_capacity == 0 ? NULL : new Slot[next_pow2(base_capacity)]),
	meta(base_capacity == 0 ? NULL : new _HashMap_::SlotMeta[next_pow2(base_capacity)]),
	capacity(base_capacity == 0 ? 0 : next_pow2(base_capacity)),
	slot_used(0),
	growth_factor(growth_factor),
	_max_load_factor(max_load_factor),
	tombstone_count(0),
	hasher(HashFunctor())
{
	if (growth_factor <= 1.0f)
		throw std::invalid_argument("growth_factor must be > 1.0");
	if (max_load_factor <= 0.0f || max_load_factor >= 1.0f)
		throw std::invalid_argument("max_load_factor must be in (0, 1)");
	std::memset(this->meta, _HashMap_::Meta::EMPTY, capacity);
}

_HashMapTemplate_
inline _HashMapDef_::HashMap(const HashMap& other) :
	table(new Slot[other.capacity]),
	meta(new _HashMap_::SlotMeta[other.capacity]),
	capacity(other.capacity),
	slot_used(other.slot_used),
	growth_factor(other.growth_factor),
	_max_load_factor(other._max_load_factor),
	tombstone_count(other.tombstone_count),
	hasher(other.hasher)
{
	for (size_t i = 0; i < other.capacity; i++)
	{
		this->meta[i] = other.meta[i];
		if (_HashMap_::Meta::is_occupied(other.meta[i]))
			this->table[i].copy_from(other.table[i]);
	}
}

_HashMapTemplate_
inline _HashMapDef_& _HashMapDef_::operator=(const HashMap& other)
{
	if (this == &other)
		return *this;
	this->clear();
	delete[] this->table;
	delete[] this->meta;
	this->table				= new Slot[other.capacity];
	this->meta				= new _HashMap_::SlotMeta[other.capacity];
	this->capacity			= other.capacity;
	this->slot_used			= other.slot_used;
	this->growth_factor		= other.growth_factor;
	this->_max_load_factor	= other._max_load_factor;
	this->tombstone_count	= other.tombstone_count;
	this->hasher			= other.hasher;
	for (size_t i = 0; i < other.capacity; i++)
	{
		this->meta[i] = other.meta[i];
		if (_HashMap_::Meta::is_occupied(other.meta[i]))
			this->table[i].copy_from(other.table[i]);
	}
	return *this;
}

_HashMapTemplate_
inline _HashMapDef_::~HashMap()
{
	this->clear();
	delete[] this->table;
	delete[] this->meta;
}

_HashMapTemplate_
inline void _HashMapDef_::insert_unsafe(const key_type& key, const mapped_type& value)
{
    size_t              hash  = this->hasher(key);
    unsigned char       h7    = _HashMap_::Meta::h7(hash);
    size_t              index = hash & (this->capacity - 1);

    while (_HashMap_::Meta::is_occupied(this->meta[index]))
        index = (index + 1) & (this->capacity - 1);

    this->table[index].construct(std::make_pair(key, value));
    this->meta[index] = h7;
    this->slot_used++;
}

_HashMapTemplate_
inline void _HashMapDef_::insert(const key_type& key, const mapped_type& value)
{
	if (this->load_factor() >= this->_max_load_factor)
		this->_rehash(next_pow2(this->capacity * this->growth_factor));
	size_t				hash	= this->hasher(key);
	_HashMap_::SlotMeta	h7		= _HashMap_::Meta::h7(hash);
	size_t				index	= this->probe(hash, h7, key);
	if (index == SIZE_MAX)
		throw std::runtime_error("Unable to insert value into an HashMap");
	if (_HashMap_::Meta::is_occupied(this->meta[index]))
		this->table[index].ptr()->second = value;
	else
	{
		if (_HashMap_::Meta::is_deleted(this->meta[index]))
			this->tombstone_count--;
		this->table[index].construct(std::make_pair(key, value));
		this->meta[index] = h7;
		this->slot_used++;
	}
}

_HashMapTemplate_
inline _HashMapType_(mapped_type)& _HashMapDef_::at(const key_type& key)
{
	size_t	index = this->probe(key);
	if (index == SIZE_MAX || !_HashMap_::Meta::is_occupied(this->meta[index]))
		throw std::runtime_error("Key entry doesn't exist in hashmap");
	return (this->table[index].ptr()->second);
}

_HashMapTemplate_
inline const _HashMapType_(mapped_type)& _HashMapDef_::at(const key_type& key) const
{
	size_t	index = this->probe(key);
	if (index == SIZE_MAX || !_HashMap_::Meta::is_occupied(this->meta[index]))
		throw std::runtime_error("Key entry doesn't exist in hashmap");
	return (this->table[index].ptr()->second);
}

_HashMapTemplate_
inline  _HashMapType_(iterator) _HashMapDef_::find(const key_type& key)
{
	size_t	index = this->probe(key);

	if (index == SIZE_MAX || !_HashMap_::Meta::is_occupied(this->meta[index]))
		return this->end();
	return iterator(this, index);
}

_HashMapTemplate_
inline _HashMapType_(const_iterator) _HashMapDef_::find(const key_type& key) const
{
	size_t	index = this->probe(key);

	if (index == SIZE_MAX || !_HashMap_::Meta::is_occupied(this->meta[index]))
		return this->end();
	return const_iterator(this, index);
}

_HashMapTemplate_
inline bool _HashMapDef_::contain(const key_type& key) const
{
	return this->find(key) != this->end();
}

_HashMapTemplate_
inline void _HashMapDef_::clear(void)
{
	for (size_t index = 0; index < this->capacity; index++)
	{
		if (_HashMap_::Meta::is_occupied(this->meta[index]))
		{
			this->table[index].destroy();
		}
		this->meta[index] = _HashMap_::Meta::EMPTY;
	}
}

_HashMapTemplate_
inline bool _HashMapDef_::erase(const key_type &key)
{
	size_t index = this->probe(key);
	if (index == SIZE_MAX || !_HashMap_::Meta::is_occupied(this->meta[index]))
		return false;
	this->table[index].destroy();
	this->meta[index] = _HashMap_::Meta::DELETED;
	this->slot_used--;
	this->tombstone_count++;
	return true;
}

_HashMapTemplate_
inline void _HashMapDef_::rehash(size_t new_capacity)
{
	new_capacity = this->slot_used > new_capacity ? this->slot_used : new_capacity;
	this->_rehash(next_pow2(new_capacity));
}

_HashMapTemplate_
inline size_t _HashMapDef_::size() const
{
	return this->slot_used;
}

_HashMapTemplate_
inline bool _HashMapDef_::empty() const
{
	return this->slot_used == 0;
}

_HashMapTemplate_
inline float _HashMapDef_::load_factor() const
{
	if (this->capacity == 0)
		return 1.0;
	return static_cast<float>(this->slot_used + this->tombstone_count) / this->capacity;
}

_HashMapTemplate_
inline float _HashMapDef_::max_load_factor() const
{
	return this->_max_load_factor;
}

_HashMapTemplate_
inline size_t _HashMapDef_::probe(const key_type& key) const
{
	size_t	hash = this->hasher(key);
	return this->probe(hash, static_cast<unsigned char>(hash & 0x7F), key);
}

_HashMapTemplate_
inline size_t _HashMapDef_::probe(size_t hash, unsigned char h7, const key_type &key) const
{
	size_t				first_tombstone = SIZE_MAX;
	size_t				index = hash & (this->capacity - 1);

	for (size_t i = 0; i < this->capacity; ++i)
	{
		_HashMap_::SlotMeta m = this->meta[index];
		if (_HashMap_::Meta::is_empty(m))
			return (first_tombstone != SIZE_MAX) ? first_tombstone : index;
		if (_HashMap_::Meta::is_deleted(m))
		{
			if (first_tombstone == SIZE_MAX)
				first_tombstone = index;
		}
		else if (m == h7 && this->table[index].ptr()->first == key)
			return index;
		index = (index + 1) & (this->capacity - 1);
	}
	return first_tombstone;
}

_HashMapTemplate_
inline void _HashMapDef_::_rehash(size_t new_capacity)
{
	_HashMap_::SlotMeta	*old_meta		= this->meta;
	Slot				*old_table		= this->table;
	size_t				old_capacity	= this->capacity;

	this->meta = new _HashMap_::SlotMeta[new_capacity];
	this->table = new Slot[new_capacity];
	std::memset(this->meta, _HashMap_::Meta::EMPTY, new_capacity);
	this->capacity = new_capacity;
	this->slot_used = 0;
	this->tombstone_count = 0;

	for (size_t i = 0; i < old_capacity; ++i)
	{
		if (_HashMap_::Meta::is_occupied(old_meta[i]))
		{
			value_type* p = old_table[i].ptr();
			this->insert_unsafe(p->first, p->second);
			old_table[i].destroy();
		}
	}
	delete[] old_table;
	delete[] old_meta;
}

# undef _HashMapDef_
# undef _HashMapTemplate_
# undef _HashMapType_

#endif // _HASHMAP_H
