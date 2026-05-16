/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   FreeList.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antoine <antoine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/16 00:26:52 by antoine           #+#    #+#             */
/*   Updated: 2026/05/16 01:52:06 by antoine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef FREELIST_HPP
# define FREELIST_HPP

# include <vector>
# include <new>
# include <cstddef>

template <typename T>
class FreeList
{
private:
    union RawStorage
    {
        char buffer[sizeof(T)];
        long double aligner;
    };
    
    std::vector<RawStorage*>    blocks;
    std::vector<T*>             freeSlots;
    size_t                      blockSize;

    void    allocateBlock()
    {
        RawStorage* newBlock = new RawStorage[blockSize];
        blocks.push_back(newBlock);
        for (size_t i = 0; i < blockSize; ++i)
        {
            freeSlots.push_back(reinterpret_cast<T*>(newBlock[i].buffer));
        }
    }
public:

    FreeList(size_t initialBlocSize = 1000) : blockSize(initialBlocSize)
    {
        allocateBlock();
    }
    ~FreeList()
    {
        for (size_t i = 0; i < blocks.size(); ++i)
        {
            delete[] blocks[i];
        }
    }

    T* acquire()
    {
        if (freeSlots.empty())
        {
            allocateBlock();
        }
        T* ptr = freeSlots.back();
        freeSlots.pop_back();
        return new (ptr) T(); 
    }
    void    release(T* obj)
    {
        if (obj != NULL)
        {
            obj->~T();
            freeSlots.push_back(obj);
        }
    }
};
# endif