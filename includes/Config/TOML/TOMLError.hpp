/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TOMLError.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/18 20:22:02 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/23 03:34:22 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _TOMLERROR_H
# define _TOMLERROR_H

# include <string>
# include <sstream>
# include <stddef.h>
# include <vector>

# include "Config/toml.hpp"

namespace toml
{

	class TOMLErrorManager
	{
	public:
		TOMLErrorManager(const std::string& str) : fileName(str), silentError(false) {};
		TOMLErrorManager() : silentError(false) {};
		~TOMLErrorManager() {};

		void	emitError(const std::string& message, size_t length, size_t line, size_t col, bool snippet = true);
		void	newSnippet();
		void	snippetAppend(char c);
		void	snippetUnget();

		bool	hasErrors() const { return this->error.error_count != 0; };
		const	ParseException&	getError() const { return this->error; };

	private:
		std::string		fileName;
		std::string		currentSnippet;
		ParseException	error;
		bool			silentError;
	};

} // namespace toml

inline void toml::TOMLErrorManager::emitError(const std::string& message, size_t length, size_t line, size_t col, bool snippet)
{
	if (this->silentError)
	{
		this->error.addError("", "", length, line, col);
		return ;
	}
	this->error.addError(message, snippet ? this->currentSnippet : "", length, line, col, this->fileName);
	this->silentError = true;
}

inline void toml::TOMLErrorManager::newSnippet()
{
	this->silentError = false;
	this->currentSnippet.clear();
}

inline void toml::TOMLErrorManager::snippetAppend(char c)
{
	this->currentSnippet += c;
}

inline void toml::TOMLErrorManager::snippetUnget()
{
	if (!this->currentSnippet.empty())
		this->currentSnippet.erase(this->currentSnippet.size() - 1);
}

inline void toml::ParseException::addError(
		const std::string& message,
		const std::string& snippet, 
		size_t length,
		int line,
		int col,
		const std::string& name)
{
	this->error_count++;
	if (message == "" && snippet == "")
		return ;
	std::ostringstream oss;
	if (name != "")
		oss << name << ':';
	oss << line << ":" << col
		<< ": Error: " << message << "\n";

	if (!snippet.empty())
	{
		oss << "	" << snippet << "\n";
		oss << "	" << std::string(col > 0 ? col - 1 : 1, ' ')
			<< "^" << std::string(length > 1 ? length - 1 : 0, '~');
	}
	this->error_string += oss.str();
}

#endif // _TOMLERROR_H