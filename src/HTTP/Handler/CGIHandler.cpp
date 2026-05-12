/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/12 14:42:30 by vdurand           #+#    #+#             */
/*   Updated: 2026/05/12 16:16:52 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils/FileSystem.hpp"
#include "HTTP/Handler/CGIHandler.hpp"
#include "CGIHandler.hpp"

static inline std::string	to_env_key(const std::string& header)
{
	std::string key = "HTTP_";
	for (std::string::const_iterator it = header.begin(); it != header.end(); ++it)
		key += (*it == '-') ? '_' : std::toupper(*it);
	return key;
}

static inline std::string	*get_header(const Request::Headers& header, const std::string& key)
{
	Request::Headers::const_iterator	it = header.find(key);
	return it != header.end() ? &it->second : NULL;
}

void	CGIHandler::setEnv(const std::string& key, const std::string& value)
{
	std::string	result = key + "=" + value;
	this->env.push_back(result.c_str());
}

# define SET_ENV_FROM_HEADER(header_key)	do { std::string	*value = get_header(header, header_key); if (value != NULL) {this->setEnv(to_env_key(header_key), *value); } } while (0)

void CGIHandler::initEnvironment()
{
	const Request::Headers&	header = this->request.getHeaders();

	this->setEnv("REQUEST_METHOD", Method::toString(this->request.method));
	this->setEnv("QUERY_STRING", this->request.query_string);
	SET_ENV_FROM_HEADER("content-type");
	SET_ENV_FROM_HEADER("content-length");
	this->setEnv("SCRIPT_NAME", this->request.path);
	this->setEnv("SCRIPT_FILENAME", this->physicalPath);
}

void CGIHandler::onCreation()
{
	if (FileSystem::exists(this->physicalPath))
}
