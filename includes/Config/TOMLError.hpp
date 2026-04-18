/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TOMLError.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/18 20:22:02 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/18 23:41:01 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _TOMLERROR_H
# define _TOMLERROR_H

# include <string>
# include <sstream>
# include <stddef.h>
# include <vector>

# include "toml.hpp"

namespace toml
{

	class TOMLErrorManager
	{
	public:
		TOMLErrorManager() : silent_error(false) {};
		~TOMLErrorManager() {};

		void	emitError(const std::string& message, size_t length, size_t line, size_t col, bool snippet = true);
		void	newSnippet();
		void	snippetAppend(char c);
		void	snippetUnget();

		bool	hasErrors() const { return this->error.error_count != 0; };
		const	ParseException&	getError() const { return this->error; };

	private:
		std::string		current_snippet;
		ParseException	error;
		bool			silent_error;
	};

} // namespace toml

inline void toml::TOMLErrorManager::emitError(const std::string& message, size_t length, size_t line, size_t col, bool snippet)
{
	if (this->silent_error)
	{
		this->error.addError("", "", length, line, col);
		return ;
	}
	this->error.addError(message, this->current_snippet, length, line, col);
	this->silent_error = true;
}

inline void toml::TOMLErrorManager::newSnippet()
{
	this->silent_error = false;
	this->current_snippet.clear();
}

inline void toml::TOMLErrorManager::snippetAppend(char c)
{
	this->current_snippet += c;
}

inline void toml::TOMLErrorManager::snippetUnget()
{
	this->current_snippet.pop_back();
}

inline void toml::ParseException::addError(const std::string& message, const std::string& snippet, size_t length, int line, int col)
{
	this->error_count++;
	if (message == "" && snippet == "")
		return ;
	std::ostringstream oss;
	oss << line << ":" << col
		<< ": error: " << message << "\n";

	if (!snippet.empty())
	{
		oss << "	" << snippet << "\n";
		oss << "	" << std::string(col > 0 ? col - 1 : 1, ' ')
			<< "^" << std::string(length > 1 ? length - 1 : 0, '~');
	}
	this->error_string += oss.str();
}

#endif // _TOMLERROR_H