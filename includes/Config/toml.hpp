/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   toml.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/10 16:31:42 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/16 18:43:49 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _TOMLDOCUMENT_H
# define _TOMLDOCUMENT_H

# include <iostream>
# include <fstream>

# include "TOMLVariant.hpp"

namespace toml
{

class Document
{
public:
	Document();
	Document(const Document& other);
	~Document();
	Document& operator=(const Document& other);

	void	from_stream(std::istream& stream, bool append = false);
	void	from_file(const std::string& path, bool append = false);

	const toml::Value&	operator[](const std::string& key) const;
	bool				contain(const std::string& key) const;
	toml::Value&		getRoot();
protected:
private:
	toml::Value	root;
};

class ParseException : public std::exception
{
public:
	ParseException() : error_count(0) {};

	size_t				error_count;
	mutable std::string	error_string;

	void	addError(const std::string& message, const std::string& snippet, size_t length, int line, int col);

	const char	*what() const throw() { return this->error_string.c_str(); };
};

} // namespace toml


#endif // _TOMLDOCUMENT_H