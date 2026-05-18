// /* ************************************************************************** */
// /*                                                                            */
// /*                                                        :::      ::::::::   */
// /*   CGIHandler.cpp                                     :+:      :+:    :+:   */
// /*                                                    +:+ +:+         +:+     */
// /*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
// /*                                                +#+#+#+#+#+   +#+           */
// /*   Created: 2026/05/12 14:42:30 by vdurand           #+#    #+#             */
// /*   Updated: 2026/05/16 20:24:01 by vdurand          ###   ########.fr       */
// /*                                                                            */
// /* ************************************************************************** */

// #include "Utils/FileSystem.hpp"
// #include "HTTP/Handler/CGIHandler.hpp"

// static inline std::string	to_env_key(const std::string& header)
// {
// 	std::string key = "HTTP_";
// 	for (std::string::const_iterator it = header.begin(); it != header.end(); ++it)
// 		key += (*it == '-') ? '_' : std::toupper(*it);
// 	return key;
// }

// static inline std::string	*get_header(const Headers& header, const std::string& key)
// {
// 	Headers::const_iterator	it = header.find(key);
// 	return it != header.end() ? &it->second : NULL;
// }

// void	CGIHandler::setEnv(const std::string& key, const std::string& value)
// {
// 	std::cout << key << "=" << value << std::endl;
// 	std::string	result = key;
// 	this->env.insert(result.c_str(), value);
// }

// # define SET_ENV_FROM_HEADER(header_key)	do { std::string	*value = get_header(header, header_key); if (value != NULL) {this->setEnv(to_env_key(header_key), *value); } } while (0)

// void CGIHandler::initEnvironment()
// {
// 	const std::string	remainder = routeResult.pathRemainder.substr(1);

// 	size_t				slash = remainder.find('/');
// 	std::string			path_info, script_name = this->routeResult.route->path;

// 	if (slash != std::string::npos)
// 	{
// 		script_name += routeResult.pathRemainder.substr(0, slash + 1);
// 		path_info = remainder.substr(slash);
// 	}

//     this->setEnv("GATEWAY_INTERFACE", CGI_VERSION);
// 	this->setEnv("SERVER_PROTOCOL",   HTTP_VERSION);

// 	this->setEnv("REQUEST_METHOD", Method::toString(this->request.method));
// 	this->setEnv("QUERY_STRING", this->request.query_string);
// 	this->setEnv("REMOTE_ADDR", this->connection.getAddress().toString());
// 	this->setEnv("SCRIPT_NAME", script_name);
// 	this->setEnv("SCRIPT_FILENAME", "");
// 	this->setEnv("PATH_INFO", path_info);

// }

// void CGIHandler::onCreation()
// {
// 	this->initEnvironment();
// }
