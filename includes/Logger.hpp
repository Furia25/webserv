/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/12 17:27:55 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/12 19:07:50 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _LOGGER_H
# define _LOGGER_H

# include "EnumClass.hpp"

# include <cstring>

# include <fstream>
# include <iostream>
# include <sstream>
# include <ctime>

# define LOG_TYPE	(INFO, WARNING, ERROR, FATAL)
ENUM_CLASS(LogLevel, LOG_TYPE, ENUM_BASIC, ENUM_LITERALS(LOG_TYPE, ENUM_BASIC, ENUM_BASIC_STRING););

class LogConfig
{
public:
	static void				setDefaultStream(std::ostream& stream);
	static void				setLogFile(const std::string& filename);
	static std::ostream&	getStream();
protected:
private:
	static std::ostream		*current_stream;
	static std::ofstream	fileStream;
};

class LogMessage
{
public:
	LogMessage();
	~LogMessage();
	template <typename T>
	LogMessage&	operator<<(const T& value);

protected:
private:
};

class Logger
{
public:

	static std::ostringstream& getBuffer();
	static LogLevel	getLevel();

	# define X(e, ...) static LogMessage	e() {level = LogLevel::e; return LogMessage();}
	M_TUPLE_FOREACH(LOG_TYPE, X)
	# undef X
protected:
	static LogLevel				level;
	static std::ostringstream	buffer;
};

template <typename T>
LogMessage&	LogMessage::operator<<(const T& value)
{
	Logger::getBuffer() << value;
	return (*this);
}

#endif // _LOGGER_H