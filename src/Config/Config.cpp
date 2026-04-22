/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/22 23:35:29 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/23 01:39:27 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config.hpp"
#include <sstream>

Config::AppConfig::AppConfig(const std::string& path)
{
	Config::Loader loader;

	toml::Document	document(path);
	toml::Variant	root = document.getRoot();
	toml::Array		server_array;

	loader.section(root, "engine", true, this->engineConfig);
	loader.section(root, "logging", true, this->loggingConfig);
	loader.array_section(root, "servers", false, server_array);

	for (size_t i = 0; i < server_array.size(); ++i)
	{
		ServerConfig server_config;
		loader.section(server_array[i], "servers[" + std::to_string(i) + "]", false, server_config);
		this->serversConfig.insert(server_config.name, server_config);
	}

	if (loader.hasErrors())
		throw Config::Exception(loader.format());
}

void Config::LoggingConfig::load(toml::Variant& table, Config::Loader& loader)
{
	loader.value_or(table, "log_file", this->log_file, std::string(""));
	std::string level_str;
	loader.value_or(table, "level", level_str, std::string(LogLevel::toString(LogLevel::INFO)));
	this->log_level = LogLevel::from(level_str);
}

void Config::EngineConfig::load(toml::Variant& table, Config::Loader& loader)
{
	loader.value_limited(table, "max_events", this->max_events, CONFIG_MAX_EVENTS, 8, 1024);
	loader.value_limited(table, "max_timeout", this->max_timeout, CONFIG_ABSOLUTE_TIMEOUT, 1, 360);
	loader.value_limited(table, "closing_timeout", this->closing_timeout, CONFIG_CLOSING_TIMEOUT, 1, 360);
	loader.value_limited(table, "read_size", this->read_size, CONFIG_READ_SIZE, 8, 4096);
	loader.value_limited(table, "max_read_limit", this->max_read_limit, CONFIG_READ_LIMIT, 8196, 32768);
}

void Config::ServerConfig::load(toml::Variant &table, Config::Loader &loader)
{
	toml::Table	errors_table;
	toml::Array	routes_array;

	loader.section(table, "limits", true,  this->limits);
	loader.array_section(table, "routes", false, routes_array);

	try { errors_table = table.take_section("errors", true).as<toml::Table>(); }
	catch (const std::exception& e) { loader.push_error("errors", e.what()); }

	loader.value(table, "host",		this->host);
	loader.value(table, "service",	 this->service);
	loader.value_or(table, "server_name", this->name,  std::string("default"));
	loader.value_or(table, "root",		this->root,  std::string("./"));

	this->loadErrors(errors_table, loader);
	this->loadRoutes(routes_array, loader);
}

void Config::ServerConfig::loadErrors(toml::Table& errors_table, Config::Loader& loader)
{
	for (toml::Table::iterator it = errors_table.begin(); it != errors_table.end(); ++it)
	{
		const std::string& key = it->first;
		try
		{
			HTTPCode code = HTTPCode::NOT_FOUND;
			try {
				code = HTTPCode::from(key);
			} catch (const std::domain_error&)
			{
				char* end_ptr = NULL;
				size_t integer = std::strtoul(key.c_str(), &end_ptr, 10);
				if (*end_ptr != '\0')
					throw std::domain_error("Unknown error code: \"" + key + "\"");
				code = static_cast<HTTPCode::E>(integer);
			}
			this->error_fallbacks.insert(code, it->second.as<std::string>());
		}
		catch (const std::exception& e)
		{
			loader.push_error("errors -> " + key, e.what());
		}
	}
}

void Config::ServerConfig::loadRoutes(toml::Array& routes_array, Config::Loader& loader)
{
	for (size_t i = 0; i < routes_array.size(); ++i)
	{
		const std::string	route_name = "routes[" + std::to_string(i) + "]";

		std::string handler_str;
		loader.value(routes_array[i], "handler", handler_str);
		HandlerType handler = HandlerType::from(handler_str);

		RouteConfig* final_route = NULL;
		switch (handler)
		{
			case HandlerType::STATIC:	final_route = new StaticConfig(); break;
			case HandlerType::CGI:		final_route = new CGIConfig(); break;
			case HandlerType::STATUS:	final_route = new StatusConfig(); break;
			case HandlerType::UPLOAD:	final_route = new UploadConfig();break;
			case HandlerType::REDIRECT:	final_route = new RedirectConfig(); break;
			default:
				loader.push_error(route_name + " -> unknown handler type", handler_str);
				continue;
		}

		loader.section(routes_array[i], route_name, false, *final_route);
		this->routes.insert(final_route->path, final_route);
	}
}

Config::ServerConfig::~ServerConfig()
{
	for (RadixTree<RouteConfig *>::iterator it = this->routes.begin(); it != this->routes.end(); ++it)
		delete it->second;
}
