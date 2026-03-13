/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/12 18:09:47 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/13 14:32:19 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"

std::ostream		*Logger::current_stream = &std::cout;
std::ofstream		Logger::file_stream;
LogLevel			Logger::min_level = LogLevel::INFO;
time_t				Logger::heartbeat_interval = 5;

LogMessage::LogMessage(LogLevel level) : level(level) {}

LogMessage::LogMessage(const LogMessage &other) : level(other.level) {}

static const char* get_level_color(LogLevel level)
{
	switch (level())
	{
		case LogLevel::FATAL:	return ANSI_BOLD ANSI_WHITE ANSI_BG_RED;
		case LogLevel::ERROR:	return ANSI_BOLD ANSI_RED;
		case LogLevel::WARNING:	return ANSI_BOLD ANSI_YELLOW;
		case LogLevel::INFO:	return ANSI_BOLD ANSI_CYAN;
		case LogLevel::DEBUG:	return ANSI_BOLD ANSI_WHITE ANSI_BG_BRIGHT_BLUE;
		default:				return ANSI_RESET;
	}
}

LogMessage::~LogMessage()
{
	if (this->level < Logger::getGlobalLevel())
		return ;
	struct timeval	tv;
	gettimeofday(&tv, NULL);

	struct tm		datetime;
	localtime_r(&tv.tv_sec, &datetime);

	char			timestr[50];
	std::strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", &datetime);

	int millis = tv.tv_usec / 1000;
	std::ostream& os = Logger::getStream();
	bool tty = isatty(STDOUT_FILENO);

	if (tty) os << ANSI_BRIGHT_BLACK;
	os << '[' << timestr << "." << std::setfill('0') << std::setw(3) << millis << "] ";

	if (tty) os << get_level_color(this->level);
 	os << LogLevel::toString(this->level()) ;
	if (tty) os << ANSI_RESET;
	os << ' ' << stream.str() << "\n";
}

void Logger::setDefaultStream(std::ostream &stream)
{
	Logger::current_stream = &stream;
}

void Logger::setLogFile(const std::string &filename)
{
	if (Logger::file_stream.is_open())
		Logger::file_stream.close();
	Logger::file_stream.open(filename.c_str(), std::ios::ate);
	if (Logger::file_stream.bad())
		throw std::runtime_error("Logger: Unable to open file: " + filename);
	Logger::current_stream = &Logger::file_stream;
}

std::ostream& Logger::getStream()
{
	return *Logger::current_stream;
}

void Logger::setGlobalLevel(LogLevel level)
{
	Logger::min_level = level;
}

LogLevel Logger::getGlobalLevel()
{
	return Logger::min_level;
}

void Logger::setHeartbeatInterval(time_t interval)
{
	Logger::heartbeat_interval = interval;
}

void Logger::heartbeat()
{
	static time_t last_beat = 0;
	time_t timestamp = std::time(NULL);
	if (timestamp - last_beat >= Logger::heartbeat_interval)
	{
		Logger::DEBUG() << "Heartbeat: Server is running...";
		last_beat = timestamp;
	}
}
