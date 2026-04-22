/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigLoader.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/22 23:45:49 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/23 01:06:46 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _CONFIGLOADER_H
# define _CONFIGLOADER_H

# include "toml.hpp"

namespace Config
{

class Loader
{
public:

	inline static std::string	format_remaining(toml::Variant& table)
	{
		std::stringstream ss;
		const toml::Table& t = table.as<toml::Table>();
		for (toml::Table::const_iterator it = t.begin(); it != t.end(); ++it)
		{
			if (it != t.begin())
				ss << ", ";
			ss << "\"" << it->first << "\"";
		}
		return ss.str();
	}

	inline void	push_error(const std::string& name, const std::string& error)
	{
		this->errors.push_back(name + " -> " + error);
	}

	template <typename TConfig>
	inline void	section(toml::Variant& root, const std::string& name, bool optional, TConfig& config)
	{
		try
		{
			toml::Variant table = root.take_section(name, optional);
			Loader child;
			config.load(table, child);
			for (std::vector<std::string>::const_iterator it = child.errors.begin(); it != child.errors.end(); ++it)
				push_error(name, *it);
			if (!table.as<toml::Table>().empty())
				this->_errors.push_back(name + " -> unexpected properties: " + format_remaining(table));
		}
		catch (const std::exception& e)
		{
			this->push_error(name, e.what();)
		}
	}

	inline void	array_section(toml::Variant& root, const std::string& key, bool optional, toml::Array& out)
	{
		try { out = root.take_section_array(key, optional).as<toml::Array>(); }
		catch (const std::exception& e) { this->push_error(key, e.what()); }
	}

	template <typename T>
	inline void	value(toml::Variant& table, const std::string& key, T& target)
	{
		try { target = table.take<T>(key); }
		catch (const std::exception& e) { this->push_error(key, e.what()); }
	}

	template <typename T>
	inline void	value_or(toml::Variant& table, const std::string& key, T& target, const T& def)
	{
		try { target = table.take_or<T>(key, def); }
		catch (const std::exception& e) { this->push_error(key, e.what()); }
	}

	template <typename T, typename TDef, typename TMin, typename TMax>
	void value_limited(toml::Variant& table, const std::string& key, T& target,
					const TDef& def, const TMin& min, const TMax& max)
	{
		try { target = take_or_limited<T>(table, key, static_cast<T>(def), static_cast<T>(min), static_cast<T>(max)); }
		catch (const std::exception& e) { this->push_error(key, e.what()); }
	}

	inline bool	hasErrors() const { return !this->errors.empty(); }

	inline std::string	format() const
	{
		std::stringstream ss;
		ss << this->errors.size() << " error(s) in config:\n";
		for (std::vector<std::string>::const_iterator it = this->errors.begin(); it != this->errors.end(); ++it)
			ss << "  - " << *it << "\n";
		return ss.str();
	}

private:
	std::vector<std::string>	errors;
};

};

#endif // _CONFIGLOADER_H