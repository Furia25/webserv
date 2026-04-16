/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigBuilder.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 18:34:43 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/16 20:04:04 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigBuilder.hpp"

template <typename T>
static void configure(const toml::Table& table, T& to_configure, const std::string& key, T& default_value, bool optional = false);

static void	configure_engine(EngineConfig& config, const toml::Table& table);
static void	configure_logging(LoggingConfig& config, const toml::Table& table);
static void	configure_server(ServerConfig& config, const toml::Table& table);

void Config::from_file(const std::string &toml_file)
{
	toml::Document	document;

	document.from_file(toml_file, false);
	configure_engine(this->engineConfig, document["engine"].as<toml::Table>());
	if (document.contain("logging"))
		configure_logging(this->loggingConfig, document["logging"].as<toml::Table>());
	if (!document.contain("servers"))
		throw Config::Exception("One virtual server at least is needed to run");
	const toml::Array&	servers = document["servers"].as<toml::Array>();
	for (toml::Array::const_iterator it = servers.begin(); it != servers.end(); ++it)
	{
		this->servers.push_back(ServerConfig());
		ServerConfig& server_config = this->servers.back();
		configure_server(server_config, it->as<toml::Table>());
	}
}

void configure_engine(EngineConfig& config, const toml::Table& table)
{
	config.max_timeout = CONFIG_ABSOLUTE_TIMEOUT;
	config.max_events = 
}

template <typename T>
Optional<std::string> configure(const toml::Table& table, T& to_configure, const std::string& key, T& default_value, bool optional)
{
	if (!table.contain(key))
	{
		if (optional)
			to_configure = default_value
		return (optional)
	}
	const toml::Value& value = table[key];
}
