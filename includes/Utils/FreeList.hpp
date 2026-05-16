/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FreeList.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/16 00:26:52 by antoine           #+#    #+#             */
/*   Updated: 2026/05/16 05:52:56 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FREELIST_HPP
# define FREELIST_HPP

# include <new>
# include <cstddef>

# include "Optional.hpp"

template <typename T>
class FreeList
{
public:
	FreeList(std::size_t blockSize = 64) : blocks(NULL), freeHead(NULL), blockSize(blockSize)
	{
		growPool();
	}

	~FreeList()
	{
		Block *block = blocks;
		while (block)
		{
			Block *next = block->next;
			delete block;
			block = next;
		}
	}

	inline void	*acquire()
	{
		if (!freeHead)
			growPool();

		Slot *slot = freeHead;
		freeHead = slot->next;

		return AlignedBuffer<T>::ptr(slot->storage); 
	}

	inline void	releaseRaw(void *ptr)
	{
		if (!ptr)
			return;

		Slot *slot = reinterpret_cast<Slot *>(ptr);
		slot->next = freeHead;
		freeHead = slot;
	}

	inline void	release(T *obj)
	{
		if (!obj)
			return;

		obj->~T();

		this->releaseRaw(obj);
	}

private:

	union Slot
	{
		typename AlignedBuffer<T>::type	storage;
		Slot							*next;
	};

	struct Block
	{
		Slot	*slots;
		Block	*next;

		Block(std::size_t n) : next(NULL)
		{
			slots = static_cast<Slot *>(::operator new(n * sizeof(Slot)));
		}

		~Block()
		{
			::operator delete(slots);
		}

	private:
		Block(const Block&);
		Block& operator=(const Block&);
	};

	Block		*blocks;
	Slot		*freeHead;
	std::size_t	blockSize;

	inline void	growPool()
	{
		Block	*new_block = new Block(blockSize);
		new_block->next = blocks;
		blocks = new_block;

		for (std::size_t i = 0; i < blockSize; ++i)
		{
			new_block->slots[i].next = freeHead;
			freeHead = &new_block->slots[i];
		}
	}

	FreeList(const FreeList&);
	FreeList& operator=(const FreeList&);
};

# endif