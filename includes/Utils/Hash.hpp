/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Hash.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/22 20:09:28 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/25 21:39:42 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _HASH_H
# define _HASH_H

# include <string>
# include <stddef.h>

/* Default Hash Trait */
template <typename Key>
struct Hash
{
	size_t	operator()(const Key& key) const;
};

#pragma region Helpers
inline size_t fnv1a(const unsigned char* data, size_t len)
{
	size_t hash = 2166136261UL;
	for (size_t i = 0; i < len; i++)
	{
		hash ^= data[i];
		hash *= 16777619UL;
	}
	return hash;
}

template <typename T>
inline size_t hash_bytes(const T& val)
{
	return fnv1a(reinterpret_cast<const unsigned char*>(&val), sizeof(T));
}

inline size_t hash_int(unsigned long long key)
{
	// Knuth multiplicative hashing
	// 2^64 / phi, phi = golden ratio
	return static_cast<size_t>(key * 11400714819323198485ULL);
}
#pragma endregion

# define HASH_INTEGRAL(type) \
template <> struct Hash<type> \
{ \
	size_t operator()(type key) const \
	{ return hash_int(key); } \
};

HASH_INTEGRAL(bool)
HASH_INTEGRAL(char)
HASH_INTEGRAL(signed char)
HASH_INTEGRAL(unsigned char)
HASH_INTEGRAL(short)
HASH_INTEGRAL(unsigned short)
HASH_INTEGRAL(int)
HASH_INTEGRAL(unsigned int)
HASH_INTEGRAL(long)
HASH_INTEGRAL(unsigned long)
HASH_INTEGRAL(long long)
HASH_INTEGRAL(unsigned long long)

# undef HASH_INTEGRAL

template <>
struct Hash<float>
{
	size_t operator()(float key) const
	{
		if (key == 0.0f) return 0;
		return hash_bytes(key);
	}
};

template <>
struct Hash<double>
{
	size_t operator()(double key) const
	{
		if (key == 0.0) return 0;
		return hash_bytes(key);
	}
};

template <>
struct Hash<long double>
{
	size_t operator()(long double key) const
	{
		if (key == 0.0L) return 0;
		return hash_bytes(key);
	}
};

template <typename T>
struct Hash<T*>
{
	size_t operator()(T* ptr) const
	{
		return static_cast<size_t>(reinterpret_cast<size_t>(ptr) >> 3);
	}
};

template <>
struct Hash<void*>
{
	size_t operator()(void* ptr) const
	{
		return static_cast<size_t>(reinterpret_cast<size_t>(ptr) >> 3);
	}
};

template <>
struct Hash<const char*>
{
	size_t operator()(const char* key) const
	{
		size_t len = 0;
		while (key[len]) len++;
		return fnv1a(reinterpret_cast<const unsigned char*>(key), len);
	}
};

template <>
struct Hash<char*>
{
	size_t operator()(char* key) const
	{
		return Hash<const char*>()(key);
	}
};

template <>
struct Hash<std::string>
{
	size_t operator()(const std::string& key) const
	{
		return fnv1a(
			reinterpret_cast<const unsigned char*>(key.data()),
			key.size()
		);
	}
};

#endif // _HASH_H