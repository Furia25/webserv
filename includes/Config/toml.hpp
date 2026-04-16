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

	toml::Value&	operator[](const std::string& key);
	toml::Value&	getRoot();
protected:
private:
	toml::Value	root;
};

} // namespace toml


#endif // _TOMLDOCUMENT_H