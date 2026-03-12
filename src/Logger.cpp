/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/12 18:09:47 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/12 19:09:53 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.hpp"

std::ostream		*LogConfig::current_stream = &std::cout;
std::ofstream		LogConfig::fileStream;
std::ostringstream	Logger::buffer;
LogLevel			Logger::level = LogLevel::INFO;

LogMessage::LogMessage() {}

LogMessage::~LogMessage()
{
	std::ostream& stream = LogConfig::getStream();
	stream << "[" << LogLevel::toString(Logger::getLevel()()) << "] " << Logger::getBuffer().str();
	Logger::getBuffer().str("");
}

void LogConfig::setDefaultStream(std::ostream &stream)
{
	LogConfig::current_stream = &stream;
}

void LogConfig::setLogFile(const std::string &filename)
{
	if (LogConfig::fileStream.is_open())
		LogConfig::fileStream.close();
	LogConfig::fileStream.open(filename.c_str(), std::ios::ate);
	if (LogConfig::fileStream.bad())
		throw std::runtime_error("Logger: Unable to open file: " + filename);
	LogConfig::current_stream = &LogConfig::fileStream;
}

std::ostream& LogConfig::getStream()
{
	return *LogConfig::current_stream;
}

std::ostringstream& Logger::getBuffer()
{
	return Logger::buffer;
}

LogLevel Logger::getLevel()
{
	return Logger::level;
}
