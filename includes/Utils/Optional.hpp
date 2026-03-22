/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Optional.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/22 14:38:55 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/22 18:10:12 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _OPTIONAL_H
# define _OPTIONAL_H

# include <stdexcept>
# include <stdint.h>
# include <stddef.h>

template <typename T>
struct alignment_of
{
	struct probe { uint8_t c; T t; };
	static const size_t value = sizeof(probe) - sizeof(T);
};

template <size_t N> struct type_with_alignment	{ typedef long double type; };
template <> struct type_with_alignment<1>		{ typedef char type; };
template <> struct type_with_alignment<2>		{ typedef short type; };
template <> struct type_with_alignment<4>		{ typedef int type; };
template <> struct type_with_alignment<8>		{ typedef double type; };

template <size_t Size, size_t Alignment>
struct aligned_storage
{
	union
	{
		uint8_t	raw[Size];
		typename type_with_alignment<Alignment>::type alignmentDummy;
	};
};

template <class T>
class Optional
{
public:
	Optional();
	Optional(const T& val);
	Optional(const Optional& other);
	~Optional();

	Optional&	operator= (const Optional& other);
	Optional&	operator= (const T& val);

	void		reset();
	T			value_or(const T& default_val) const;

	void		swap(Optional& other);

	T&			value();
	const T&	value() const;

	bool		has_value() const;
	operator	bool() const;

	T&			operator* ();
	const T&	operator* () const;

	T*			operator->();
	const T*	operator->() const;

protected:
private:
	aligned_storage<sizeof(T), alignment_of<T>::value> buffer;
	bool		initialized;

	T*			ptr();
	const T*	ptr() const;

	void		construct(const T& val);
	void		destroy();
};

template <class T>
inline Optional<T>::Optional() : initialized(false) {}

template <class T>
inline Optional<T>::Optional(const T& val) : initialized(false)
{
	this->construct(val);
}

template <class T>
inline Optional<T>::Optional(const Optional& other) : initialized(false)
{
	if (other.initialized)
		this->construct(*other.ptr());
}

template <class T>
inline Optional<T>::~Optional()
{
	if (this->initialized)
		this->destroy();
}

template <class T>
inline Optional<T>& Optional<T>::operator=(const Optional& other)
{
	if (&other == this)
		return (*this);
	if (this->initialized)
		this->destroy();
	if (other.initialized)
		this->construct(*other.ptr());
	return (*this);
}

template <class T>
inline Optional<T>& Optional<T>::operator=(const T& val)
{
	if (this->initialized)
		this->destroy();
	this->construct(val);
	return (*this);
}

template <class T>
inline void Optional<T>::reset()
{
	if (this->initialized)
		this->destroy();
}

template <class T>
inline T Optional<T>::value_or(const T& default_val) const
{
	if (this->initialized)
		return (*this->ptr());
	return (default_val);
}

template <class T>
inline void Optional<T>::swap(Optional& other)
{
	if (this->initialized && other.initialized)
	{
		T temp = *this->ptr();
		*this->ptr() = *other.ptr();
		*other.ptr() = temp;
	}
	else if (this->initialized)
	{
		other.construct(*this->ptr());
		this->destroy();
	}
	else if (other.initialized)
	{
		this->construct(*other.ptr());
		other.destroy();
	}
}

template <class T>
inline T& Optional<T>::value()
{
	if (!this->initialized)
		throw std::runtime_error("Optional is empty");
	return (*this->ptr());
}

template <class T>
inline const T &Optional<T>::value() const
{
	if (!this->initialized)
		throw std::runtime_error("Optional is empty");
	return (*this->ptr());
}

template <class T>
inline bool Optional<T>::has_value() const
{
	return this->initialized;
}

template <class T>
inline Optional<T>::operator bool() const
{
	return this->initialized;
}

template <class T>
inline T& Optional<T>::operator*()
{
	return *this->ptr();
}

template <class T>
inline const T& Optional<T>::operator*() const
{
	return *this->ptr();
}

template <class T>
inline T *Optional<T>::operator->()
{
	return this->ptr();
}

template <class T>
inline const T *Optional<T>::operator->() const
{
	return this->ptr();
}

template <class T>
inline T *Optional<T>::ptr()
{
	return reinterpret_cast<T *>(&this->buffer.raw);
}

template <class T>
inline const T *Optional<T>::ptr() const
{
	return reinterpret_cast<const T *>(&this->buffer.raw);
}

template <class T>
inline void Optional<T>::construct(const T &val)
{
	new (&this->buffer.raw) T(val);
	this->initialized = true;
}

template <class T>
inline void Optional<T>::destroy()
{
	this->ptr()->~T();
	initialized = false;
}

template <class T>
bool operator==(const Optional<T>& a, const Optional<T>& b)
{
	if (a.has_value() != b.has_value())
		return false;
	if (!a.has_value())
		return true;
	return *a == *b;
}

template <class T>
bool operator!=(const Optional<T>& a, const Optional<T>& b)
{
	return !(a == b);
}

template <typename T>
bool operator==(const Optional<T>& optional, const T& val)
{
	return optional.has_value() && *opt == val;
}

template <typename T>
bool operator==(const T& val, const Optional<T>& optional)
{
	return optional == val;
}

template <typename T>
bool operator!=(const Optional<T>& optional, const T& val) { return !(optional == val); }

template <typename T>
bool operator!=(const T& val, const Optional<T>& optional) { return !(optional == val); }

#endif // _OPTIONAL_H
