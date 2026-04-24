/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/22 23:35:29 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/24 15:35:52 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config/Config.hpp"
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

	for (size_t index = 0; index < server_array.size(); ++index)
	{
		ServerConfig		server_config;
		std::stringstream	ss;
		ss << "servers[" << index << ']';
		loader.direct_section(server_array[index], ss.str(), server_config);
		this->serversConfig.insert(server_config.name, server_config);
	}

	if (loader.hasErrors())
		throw Config::Exception(loader.format());
}

void Config::LoggingConfig::load(toml::Variant& table, Config::Loader& loader)
{
	loader.value_or(table, "log_file", this->log_file, std::string(""));
	loader.value_limited_or(table, "tick_interval", this->tick_interval, CONFIG_TICK_INTERVAL, 0, 10000000);
	std::string level_str;
	loader.value_or(table, "level", level_str, std::string(LogLevel::toString(LogLevel::INFO)));
	this->log_level = LogLevel::from(level_str);
}

void Config::EngineConfig::load(toml::Variant& table, Config::Loader& loader)
{
	loader.value_limited_or(table, "max_events", this->max_events, CONFIG_MAX_EVENTS, 8, 1024);
	loader.value_limited_or(table, "max_timeout", this->max_timeout, CONFIG_ABSOLUTE_TIMEOUT, 1, 360);
	loader.value_limited_or(table, "closing_timeout", this->closing_timeout, CONFIG_CLOSING_TIMEOUT, 1, 360);
	loader.value_limited_or(table, "read_size", this->read_size, CONFIG_READ_SIZE, 8, 4096);
	loader.value_limited_or(table, "max_read_limit", this->max_read_limit, CONFIG_READ_LIMIT, 8196, 32768);
}

void Config::ServerConfig::load(toml::Variant& table, Config::Loader& loader)
{
	toml::Table	errors_table;
	toml::Array	routes_array;

	loader.value_limited_or(table, "max_body_size", this->max_body_size, CONFIG_BODY_SIZE, 0, UINT64_MAX);
	loader.array_section(table, "routes", false, routes_array);

	try { errors_table = table.take_section("errors", true).as<toml::Table>(); }
	catch (const std::exception& e) { loader.push_error("errors", e.what()); }

	loader.value(table, "host", this->host);
	loader.value(table, "service", this->service);
	loader.value_or(table, "server_name", this->name, std::string("default"));
	loader.value_or(table, "root", this->root, std::string("./"));

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
	for (size_t index = 0; index < routes_array.size(); ++index)
	{
		std::stringstream	ss;
		ss << "routes[" << index << ']';
		const std::string	route_name = ss.str();

		std::string handler_str;
		loader.value(routes_array[index], "handler", handler_str);
		HandlerType handler = HandlerType::from(handler_str);

		RouteConfig	*final_route = NULL;
		switch (handler)
		{
			case HandlerType::STATIC:	final_route = new StaticConfig(this); break;
			case HandlerType::CGI:		final_route = new CGIConfig(this); break;
			case HandlerType::STATUS:	final_route = new StatusConfig(this); break;
			case HandlerType::UPLOAD:	final_route = new UploadConfig(this);break;
			case HandlerType::REDIRECT:	final_route = new RedirectConfig(this); break;
			default:
				loader.push_error(route_name, "unknown handler type -> " + handler_str);
				continue;
		}
		final_route->addRef();
		loader.direct_section(routes_array[index], route_name, *final_route);
		this->routes.insert(final_route->path, final_route);
	}
}

Config::ServerConfig::ServerConfig(const ServerConfig& other) : routes(other.routes)
{
	for (RadixTree<RouteConfig *>::iterator it = this->routes.begin(); it != this->routes.end(); ++it)
		it->second->addRef();
}

Config::ServerConfig& Config::ServerConfig::operator=(const ServerConfig& other)
{
	if (this != &other)
		return *this;
	for (RadixTree<RouteConfig*>::iterator it = this->routes.begin(); it != this->routes.end(); ++it)
		it->second->release();
	this->routes = other.routes;
	for (RadixTree<RouteConfig*>::iterator it = this->routes.begin(); it != this->routes.end(); ++it)
		it->second->addRef();

	return *this;
}

Config::ServerConfig::~ServerConfig()
{
	for (RadixTree<RouteConfig *>::iterator it = this->routes.begin(); it != this->routes.end(); ++it)
		it->second->release();
}

void Config::RouteConfig::load(toml::Variant& table, Config::Loader& loader)
{
	loader.value(table, "path", this->path);
	loader.value_or(table, "alias", this->alias, std::string(""));
	loader.value_or(table, "root", this->root, server_config->root);
	loader.value_limited_or(table, "max_body_size", this->max_body_size, server_config->max_body_size, 0, UINT64_MAX);
	this->loadAllowedMethod(table, loader);
	this->loadBase(table, loader);
}

void Config::RouteConfig::loadAllowedMethod(toml::Variant& table, Config::Loader& loader)
{
	toml::Array methods;
	loader.value(table, "methods", methods);

	for (size_t index = 0; index < Method::length; ++index)
		this->method_allowed[index] = false;
	for (size_t index = 0; index < methods.size(); ++index)
	{
		try {
			const std::string&	key = methods[index].as<std::string>();
			Method				method = Method::from(key);
			this->method_allowed[method] = true;
		} catch (const std::exception& e)
		{
			std::stringstream	ss;
			ss << "unknown method" << " at index " << index;
			loader.push_error("methods", ss.str());
		}
	}
}

void Config::StaticConfig::loadBase(toml::Variant& table, Config::Loader& loader)
{
	loader.value(table, "index", this->index);
	loader.value(table, "autoindex", this->autoindex);
}

void Config::UploadConfig::loadBase(toml::Variant& table, Config::Loader& loader)
{
	loader.value_or(table, "upload_store", this->upload_store, this->root);
	loader.value_or(table, "allow_overwrite", this->allow_overwrite, false);
	this->loadAllowedExtensions(table, loader);
}

void Config::UploadConfig::loadAllowedExtensions(toml::Variant& table, Config::Loader& loader)
{
	toml::Array extensions;
	loader.value(table, "allowed_extensions", extensions);
	for (size_t index = 0; index < extensions.size(); ++index)
	{
		try {
			const std::string&	key = extensions[index].as<std::string>();
			MIME				mime_type = MIME::from(key);
			this->allowed_extensions.push_back(mime_type);
		} catch (const std::exception& e)
		{
			std::stringstream	ss;
			ss << "unknown mime type" << " at index " << index;
			loader.push_error("allowed_extensions", ss.str());
		}
	}
}

void Config::RedirectConfig::loadBase(toml::Variant& table, Config::Loader& loader)
{
	loader.value(table, "location", this->redirect_location);

	std::string code_str;
	loader.value(table, "status", code_str);
	this->status = HTTPCode::from(code_str);
}

void Config::CGIConfig::loadBase(toml::Variant &table, Config::Loader &loader)
{
	loader.value(table, "cgi_bin", this->bin);
	loader.value_or(table, "interpreter", this->interpreter, std::string(""));
	loader.value_limited(table, "cgi_timeout", this->timeout, 0, 360);

	toml::Table temp_env;
	loader.value_or(table, "env", temp_env, toml::Table());
	size_t index = 0;
	for (toml::Table::iterator it = temp_env.begin(); it != temp_env.end() ; ++it)
	{
		std::string			value;
		toml::Value::Type	type = it->second.getType();
		if (type == toml::Value::STRING)
			value = it->second.as<std::string>();
		else if (type == toml::Value::INTEGER || type == toml::Value::FLOATING)
		{
			std::stringstream	ss;
			ss << (type == toml::Value::INTEGER ? it->second.as<long long>() : it->second.as<double>());
			value = ss.str();
		}
		else
		{
			std::stringstream	ss;
			ss << "Invalid environment variable at index " <<  index
				<< ": Only strings, integers and floats can be used as environment variables";
			loader.push_error("env", ss.str());
			index++;
			continue ;
		}
		this->env.insert(it->first, value);
		index++;
	}
}
