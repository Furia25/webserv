/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/12 17:27:55 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/15 21:20:37 by vdurand          ###   ########.fr       */
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
# include <sys/time.h>
# include <iomanip>
# include <unistd.h>

# include "Debugging.hpp"
# include "ansi_sequence.hpp"

# define LOG_TYPE	(INFO, WARNING, ERROR, FATAL, DEBUG)
ENUM_CLASS(LogLevel, LOG_TYPE, ENUM_BASIC, ENUM_LITERALS(LOG_TYPE, ENUM_BASIC, ENUM_BASIC););

class LogMessage
{
public:
	LogMessage(LogLevel level);
	LogMessage(const LogMessage& other);
	~LogMessage();

	template <typename T>
	LogMessage&	operator<<(const T& value);

protected:
private:
	LogLevel			level;
	std::ostringstream	stream;
};

class Logger
{
public:
	typedef void (*TickCallback)(void*);

	static void					setDefaultStream(std::ostream& stream);
	static void					setLogFile(const std::string& filename);
	static std::ostream&		getStream();
	static void					setGlobalLevel(LogLevel level);
	static LogLevel				getGlobalLevel();
	static void					tick();
	static void					setTickInterval(time_t interval);
	static void					setTickCallback(TickCallback callback, void *context);

	# define X(e, ...) static LogMessage	e() {return LogMessage(LogLevel::e);}
	M_TUPLE_FOREACH(LOG_TYPE, X)
	# undef X
protected:
private:
	static LogLevel			min_level;
	static std::ostream		*current_stream;
	static std::ofstream	file_stream;
	static time_t			tick_interval;
	static TickCallback		callback;
	static void				*callback_context;
};

template <typename T>
LogMessage&	LogMessage::operator<<(const T& value)
{
	stream << value;
	return (*this);
}

#endif // _LOGGER_H