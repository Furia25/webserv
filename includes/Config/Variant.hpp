/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Variant.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/31 11:51:56 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/31 12:26:18 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _VARIANT_H
# define _VARIANT_H

# include <cstring>
# include <vector>

# include "Optional.hpp"
# include "HashMap.hpp"

class Variant
{
public:
	enum Type { STRING, BOOLEAN, INTEGER, FLOAT, DATETIME, ARRAY, TABLE, NONE };

	typedef std::vector<Variant>			Array;
	typedef HashMap<std::string, Variant>	Table;

	Variant();
	Variant(bool value);
	Variant(long long value);
	Variant(double value);
	Variant(const std::string& value);
	Variant(const char *value);
	Variant(const Array& value);
	Variant(const Table& value);

	Variant(const Variant& other);

	~Variant();

	Variant& operator=(const Variant& other);

	template <typename T> T&       as();
	template <typename T> const T& as() const;
protected:
private:
	Type	type;
	union Data
	{
		RawStorage<bool>		boolean;
		RawStorage<double>		floating;
		RawStorage<long long>	integer;
		RawStorage<std::string>	string;
		RawStorage<Array>		array;
		RawStorage<Table>		table;
	}	data;
};

#endif // _VARIANT_H