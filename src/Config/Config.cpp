/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/22 23:35:29 by vdurand           #+#    #+#             */
/*   Updated: 2026/05/18 13:36:43 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Config/Config.hpp"
#include "Utils/IntegerUtils.hpp"
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
		ServerConfig	*server_config = new ServerConfig();
		try
		{
			std::stringstream	ss;
			ss << "servers[" << index << ']';
			loader.direct_section(server_array[index], ss.str(), *server_config);
			for (size_t index = 0; index < server_config->bindings.size(); ++index)
			{
				const std::pair<std::string, port_t> binding = server_config->bindings[index];
				const std::string&	host = binding.first;
				port_t				port = binding.second;

				if (!this->serversMap.contain(port))
					this->serversMap.insert(port, RadixTree<ServerConfig *>());
				else if (this->serversMap.at(port).count(host) != 0)
					throw std::runtime_error("Can't redefine binding in server "
						+ server_config->name + " for " + host + ":" + IntegerUtils::itoa(port));
				this->serversMap.at(port).insert(host, server_config);
			}
			this->servers.push_back(server_config);
		}
		catch (const std::exception& e)
		{
			delete server_config;
			loader.push_error(server_config->name, e.what());
			break ;
		}
	}

	if (loader.hasErrors())
	{
		for (std::vector<ServerConfig *>::iterator it = this->servers.begin(); it != this->servers.end(); ++it)
			delete *it;
		throw Config::Exception(loader.format());
	}
}

Config::AppConfig::~AppConfig()
{
	for (std::vector<ServerConfig *>::iterator it = this->servers.begin(); it != this->servers.end(); ++it)
		delete *it;
}

void Config::LoggingConfig::load(toml::Variant &table, Config::Loader &loader)
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

Config::ServerConfig::~ServerConfig()
{
	for (RadixTree<RouteConfig *>::iterator it = this->routes.begin(); it != this->routes.end(); ++it)
		delete it->second;
}

static inline void loadCookies(HashMap<std::string, Config::CookieConfig>& cookies_map,
						toml::Variant& table, Config::Loader& loader);

void Config::ServerConfig::load(toml::Variant& table, Config::Loader& loader)
{
	toml::Array	bindings_array;
	loader.array_section(table, "bindings", false, bindings_array);

	toml::Array	routes_array;
	loader.array_section(table, "routes", false, routes_array);

	toml::Table	errors_table;
	try { errors_table = table.take_section("errors", true).as<toml::Table>(); }
	catch (const std::exception& e) { loader.push_error("errors", e.what()); }

	loader.value_or(table, "server_name", this->name, std::string("default"));
	loader.value_or(table, "root", this->root, std::string("./"));
	loader.value_limited_or(table, "max_body_size", this->max_body_size, CONFIG_BODY_SIZE, 0, UINT64_MAX);

	this->loadBindings(bindings_array, loader);
	this->loadErrors(errors_table, loader);
	loadCookies(this->cookies, table, loader);
	this->loadRoutes(routes_array, loader);
}

void Config::ServerConfig::loadBindings(toml::Array& bindings_array, Config::Loader& loader)
{
	for (size_t index = 0; index < bindings_array.size(); ++index)
	{
		Loader child;
		std::stringstream	ss;
		ss << "bindings[" << index << ']';
		const std::string	bindings_name = ss.str();

		std::string host, service;
		child.value(bindings_array[index], "host", host);
		child.value(bindings_array[index], "service", service);
		port_t port = AddressResolver::getPortFromService(service);
		if (port == 0)
			child.push_error("Unable to resolve service", "invalid port or service name");
		this->bindings.push_back(std::make_pair(host, port));
		for (std::vector<std::string>::const_iterator it = child.errors.begin(); it != child.errors.end(); ++it)
			loader.push_error(bindings_name, *it);
	}
}

void Config::ServerConfig::loadErrors(toml::Table& errors_table, Config::Loader& loader)
{
	for (toml::Table::iterator it = errors_table.begin(); it != errors_table.end(); ++it)
	{
		const std::string& key = it->first;
		try
		{
			HTTPCode code = HTTPCode::fromLiteral(key);
			this->error_fallbacks.insert(code, it->second.as<std::string>());
		}
		catch (const std::exception& e)
		{
			loader.push_error("errors -> " + key, e.what());
		}
	}
}

static inline void loadCookies(HashMap<std::string, Config::CookieConfig>& cookies_map,
						toml::Variant& table, Config::Loader& loader)
{
	toml::Variant	cookies = table.take_or("cookies", toml::Variant());
	if (cookies.isNone())
		return ;
	toml::Table&	cookies_table = cookies.as<toml::Table>();
	size_t			cookie_num = 0;
	for (toml::Table::iterator it = cookies_table.begin(); it != cookies_table.end(); ++it)
	{
		const std::string	cookie_name = "Cookie \"" + it->first + "\"";

		if (it->second.isImplicit() || !it->second.isHeader())
		{
			loader.push_error(cookie_name, "Cookies need to be defined as explicit headers for example: "
				"[servers.routes.cookies.foo] for actual route or [servers.cookies.foo] for server global cookies");
			continue ;
		}

		try
		{
			Config::Loader			cookie_loader;
			Config::CookieConfig	cookie_config;
			toml::Table&	actual_cookie_table = it->second.as<toml::Table>();

			cookie_config.name = it->first;
			cookie_config.load(it->second, cookie_loader);
			cookies_map.insert(it->first, cookie_config);
			for (std::vector<std::string>::const_iterator error_it = cookie_loader.errors.begin(); error_it != cookie_loader.errors.end(); ++error_it)
				loader.push_error(cookie_name, *error_it);
			if (!actual_cookie_table.empty())
				loader.push_error(cookie_name, "unexpected properties: " + Config::Loader::format_remaining(it->second));
		} catch (const std::exception& e)
		{
			loader.push_error(cookie_name, e.what());
		}
		cookie_num++;
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
		final_route->handler = handler;
		try
		{
			loader.direct_section(routes_array[index], route_name, *final_route);
			if (this->routes.count(final_route->path) != 0)
				throw std::runtime_error("Can't redefine \""
					+ final_route->path + "\" already exist");
			this->routes.insert(final_route->path, final_route);
		}
		catch(const std::exception& e)
		{
			delete final_route;
			loader.push_error(route_name, e.what());
			break ;
		}
	}
}

void Config::RouteConfig::load(toml::Variant& table, Config::Loader& loader)
{
	loader.value(table, "path", this->path);
	loader.value_or(table, "alias", this->alias, std::string(""));
	loader.value_or(table, "root", this->root, server_config->root);
	loader.value_limited_or(table, "max_body_size", this->max_body_size, server_config->max_body_size, 0, UINT64_MAX);
	this->loadAllowedMethod(table, loader);
	this->cookies = this->server_config->cookies;
	loadCookies(this->cookies, table, loader);
	this->loadChild(table, loader);
}

void Config::RouteConfig::loadAllowedMethod(toml::Variant &table, Config::Loader &loader)
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

void Config::CookieConfig::load(toml::Variant& table, Config::Loader& loader)
{
	if (table.as<toml::Table>().contain("max_age"))
		loader.value_limited<int64_t>(table, "max_age", this->max_age, 0, 34560000);
	else
		this->max_age = -1;

	std::string	temp_samesite;
	loader.value_or<std::string>(table, "same_site", temp_samesite, "");
	if (temp_samesite == "")
		this->same_site = Cookie::SameSite::LAX;
	else
	{
		try { this->same_site = Cookie::SameSite::from(temp_samesite); }
		catch (const std::exception& e)
		{
			loader.push_error("same_site", "unknown enum value needs LAX, NONE or STRICT");
		}
	}
	loader.value_or<bool>(table, "http_only", this->http_only, false);
	loader.value_or<bool>(table, "generate", this->generate, false);

	if (table.as<toml::Table>().contain("generation_length") && this->generate == false)
		loader.push_error("generation_length", "generation_length requires generate = true");
	loader.value_limited_or<size_t>(table, "generation_length", this->generation_length, 16, 16, 64);

	loader.value_or<std::string>(table, "default_value", this->default_value, "");
	loader.value_or<bool>(table, "required", this->required, this->default_value == "" && !this->generate);
}

void Config::StaticConfig::loadChild(toml::Variant& table, Config::Loader& loader)
{
	loader.value(table, "index", this->index);
	loader.value_or(table, "autoindex", this->autoindex, false);
}

void Config::UploadConfig::loadChild(toml::Variant& table, Config::Loader& loader)
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

void Config::RedirectConfig::loadChild(toml::Variant& table, Config::Loader& loader)
{
	loader.value(table, "location", this->redirect_location);

	std::string code_str;
	loader.value(table, "status", code_str);

	try { this->status = HTTPCode::fromLiteral(code_str);}
	catch (const std::exception& e)
	{
		loader.push_error("status", "Invalid error code, literal or code needed");
	}
}

void Config::CGIConfig::loadChild(toml::Variant& table, Config::Loader& loader)
{
	loader.value_or(table, "default_bin", this->default_bin, std::string(""));
	loader.value_or(table, "interpreters", this->interpreters, toml::Table());
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

void Config::StatusConfig::loadChild(toml::Variant &table, Config::Loader &loader)
{
	loader.value_or<bool>(table, "show_start_time", this->start_time, false);
	loader.value_or<bool>(table, "show_server_info", this->server_info, false);
	loader.value_or<bool>(table, "show_connection_info", this->connection_info, false);
	loader.value_or<bool>(table, "show_request_info", this->request_info, false);
	loader.value_or<bool>(table, "show_timestamp", this->timestamp, false);
}
