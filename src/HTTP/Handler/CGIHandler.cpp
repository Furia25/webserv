/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/12 14:42:30 by vdurand           #+#    #+#             */
/*   Updated: 2026/05/18 04:03:00 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils/FileSystem.hpp"
#include "HTTP/Handler/CGIHandler.hpp"
#include "Server/TCPServer.hpp"

CGIHandler::~CGIHandler()
{
	if (registered)
		this->connection.getServer().removePollEvent(*this, this.);
}

static inline std::string	to_env_key(const std::string& header)
{
	std::string key = "HTTP_";
	for (std::string::const_iterator it = header.begin(); it != header.end(); ++it)
		key += (*it == '-') ? '_' : std::toupper(*it);
	return key;
}

static inline std::string	*get_header(const Headers& header, const std::string& key)
{
	Headers::const_iterator	it = header.find(key);
	return it != header.end() ? &it->second : NULL;
}

void CGIHandler::initPaths()
{
	const std::string&	remainder = routeResult.pathRemainder;
	size_t				slash = remainder.find('/', 1);

	this->scriptName = remainder.substr(0, slash);
	if (slash != std::string::npos)
		this->pathInfo = remainder.substr(slash);
	if (this->scriptName.empty() || this->scriptName == "/")
		this->scriptName = "/" + this->CGIConfig.default_bin;

	this->scriptFilename = this->routeResult.basePath + this->scriptName;
}

inline void CGIHandler::initEnvironment()
{
	HashMap<std::string, std::string> env;
	const Headers&	headers = this->request.getHeaders();

	env.insert("SERVER_SOFTWARE", SERV_NAME "/" SERV_VERSION);
	env.insert("GATEWAY_INTERFACE", CGI_VERSION);

	env.insert("SERVER_PROTOCOL", HTTP_VERSION);
	env.insert("SERVER_NAME", this->routeResult.hostStr);
	env.insert("SERVER_PORT", IntegerUtils::itoa(this->connection.getOriginPort()));

	env.insert("REQUEST_METHOD", Method::toString(this->request.method));
	env.insert("PATH_INFO", this->pathInfo);

	env.insert("PATH_TRANSLATED", this->pathInfo.empty() ? ""
		: this->routeResult.basePath + this->pathInfo);

	env.insert("SCRIPT_NAME", this->scriptName);

	env.insert("SCRIPT_FILENAME", this->scriptFilename);
	env.insert("QUERY_STRING", this->request.query_string);

	env.insert("REMOTE_ADDR", this->connection.getAddress().toString());

	const std::string	*value = get_header(headers, HEADER_CONTENT_TYPE);
	if (value != NULL)
		env.insert("CONTENT_TYPE", *value);

	size_t	body_size = this->body.getSize();
	if (body_size != 0)
		env.insert("CONTENT_LENGTH", IntegerUtils::itoa(body_size));

	/*Insert request header*/
	for (Headers::const_iterator it = headers.begin(); it != headers.end(); ++it)
	{
		if (it->first == HEADER_CONTENT_LENGTH || it->first == HEADER_CONTENT_TYPE)
			continue ;
		env.insert(to_env_key(it->first), it->second);
	}

	/*Insert CGI default config*/
	for (HashMap<std::string, std::string>::const_iterator it = this->CGIConfig.env.begin();
			it != this->CGIConfig.env.end(); ++it)
		env.insert(it->first, it->second);

	/*Flatten the hashmap*/
	for (HashMap<std::string, std::string>::iterator it = env.begin();
			it != env.end(); ++it)
	{
		std::string	flattened;

		flattened.reserve(it->first.size() + it->second.size() + 1);
		flattened += it->first;
		flattened += "=";
		flattened += it->second;
		this->envFlat.push_back(flattened);
	}
}

void CGIHandler::onCreation()
{
	this->initPaths();
	this->initEnvironment();

	if (!FileSystem::exists(this->scriptFilename))
		throw HTTPException(HTTPCode::NOT_FOUND, "Script file not found");

	#if HTTP_DEBUG == true
	for (size_t index = 0; index < this->envFlat.size(); ++index)
		std::cout << this->envFlat[index] << "\n";
	#endif	

	if (!FileSystem::isExecutable(this->scriptFilename))
		throw HTTPException(HTTPCode::FORBIDDEN, "File not executable");

}
