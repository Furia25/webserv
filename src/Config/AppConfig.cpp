/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AppConfig.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/22 17:50:57 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/22 19:01:43 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "AppConfig.hpp"
#include <sstream>

#define CHECK_EMPTY(variant) if (!variant.as<toml::Table>().empty()) {throw std::runtime_error("Unexpected property in config"); }

template <typename T>
inline static T take_or_limited(toml::Variant& table, const std::string& key, const T& def, const T& min)
{
	T	value = table.take_or(key, def);
	if (value < min)
	{
		std::stringstream	ss;
		ss << key << " must be between " << min << " and " << max;
		throw std::runtime_error(ss.str());
	}
}

template <typename T>
inline static T take_or_limited(toml::Variant& table, const std::string& key, const T& def, const T& min, const T& max)
{
	T	value = table.take_or(key, def);
	if (value < min || value > max)
	{
		std::stringstream	ss;
		ss << key << " must be between " << min << " and " << max;
		throw std::runtime_error(ss.str());
	}
}

void Config::load(toml::Document& document)
{
	toml::Variant	root = document.getRoot();
	try
	{
		toml::Variant	engine_table = root.take_section("engine", true);
		toml::Variant	logging_table = root.take_section("logging", true);
		toml::Array		server_array = root.take_section_array("servers", false).as<toml::Array>();
		CHECK_EMPTY(root);

		this->engineConfig.load(engine_table);
		this->loggingConfig.load(logging_table);

		for (toml::Array::iterator it = server_array.begin(); it != server_array.end(); ++it)
		{
			ServerConfig	server_config;
			server_config.load(*it);
			this->serversConfig.insert(server_config.name, server_config);
		}
	}
	catch (const std::exception& e)
	{
		throw Config::Exception(e.what());
	}
}

void LoggingConfig::load(toml::Variant& table)
{
	this->log_file = table.take_or<std::string>("log_file", "");
	std::string level_str = table.take_or<std::string>("level", LogLevel::toString(LogLevel::INFO));
	this->log_level = LogLevel::from(level_str);
	CHECK_EMPTY(table);
}

void EngineConfig::load(toml::Variant& table)
{
	this->max_events = take_or_limited<int>(table, "max_events", CONFIG_MAX_EVENTS, 8, 1024);
	this->max_timeout = take_or_limited<int>(table, "max_timeout", CONFIG_ABSOLUTE_TIMEOUT, 1, 360);
	this->closing_timeout = take_or_limited<int>(table, "closing_timeout", CONFIG_CLOSING_TIMEOUT, 1, 360);
	this->read_size = take_or_limited<int>(table, "read_size", CONFIG_READ_SIZE, 8, 4096);
	this->max_read_limit = take_or_limited<int>(table, "max_read_limit", CONFIG_READ_LIMIT, 8196, 32768);
	CHECK_EMPTY(table);
}

void ServerConfig::load(toml::Variant& table)
{
	toml::Variant	limits_table = table.take_section("limits", true);
	toml::Table		errors_table = table.take_section("errors", true).as<toml::Table>();
	toml::Array		routes_array = table.take_section_array("routes", false).as<toml::Array>();

	this->host = table.take<std::string>("host");
	this->service = table.take<std::string>("service");
	this->service = table.take_or<std::string>("server_name", "default");
	this->root = table.take_or<std::string>("root", "./");
	CHECK_EMPTY(table);

	this->limits.load(limits_table);
	this->load_errors(errors_table);
}

void ServerConfig::load_errors(toml::Table& errors_table)
{
	for (toml::Table::iterator it = errors_table.begin(); it != errors_table.end(); ++it)
	{
		const std::string&	key = it->first;
		HTTPCode			code = HTTPCode::NOT_FOUND;
		try {
			code = HTTPCode::from(key);
		} catch (const std::domain_error& e)
		{
			char *end_ptr = NULL;
			size_t integer = std::strtoul(key.c_str(), &end_ptr, 10);
			if (*end_ptr != '\0')
				throw std::domain_error("Unknown error in error table: \"" + key + '\"');
			code = static_cast<HTTPCode::E>(integer);
		}
		this->error_fallbacks.insert(code, it->second.as<std::string>());
	}
}
