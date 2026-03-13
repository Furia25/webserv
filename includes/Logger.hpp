/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/12 17:27:55 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/13 14:28:57 by vdurand          ###   ########.fr       */
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

# include "ansi_sequence.hpp"

# define LOG_TYPE	(INFO, WARNING, ERROR, FATAL, DEBUG)
ENUM_CLASS(LogLevel, LOG_TYPE, ENUM_BASIC, ENUM_LITERALS(LOG_TYPE, ENUM_BASIC, ENUM_BASIC_STRING););

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
	static void					setDefaultStream(std::ostream& stream);
	static void					setLogFile(const std::string& filename);
	static std::ostream&		getStream();
	static void					setGlobalLevel(LogLevel level);
	static LogLevel				getGlobalLevel();
	static void					heartbeat();
	static void					setHeartbeatInterval(time_t interval);

	# define X(e, ...) static LogMessage	e() {return LogMessage(LogLevel::e);}
	M_TUPLE_FOREACH(LOG_TYPE, X)
	# undef X
protected:
	static LogLevel				min_level;
	static std::ostream			*current_stream;
	static std::ofstream		file_stream;
	static time_t				heartbeat_interval;
};

template <typename T>
LogMessage&	LogMessage::operator<<(const T& value)
{
	stream << value;
	return (*this);
}

#endif // _LOGGER_H