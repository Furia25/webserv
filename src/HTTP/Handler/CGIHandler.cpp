/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/12 14:42:30 by vdurand           #+#    #+#             */
/*   Updated: 2026/05/21 01:03:12 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils/FileSystem.hpp"
#include "HTTP/Handler/CGIHandler.hpp"
#include "Server/TCPServer.hpp"
#include <sys/wait.h>
#include "CGIHandler.hpp"

#define SAFE_CLOSE(fd) do {if (fd != -1) {close(fd); fd = -1;}} while (0)

CGIHandler::~CGIHandler()
{
	if (registered)
	{
		if (this->pipeIn[1] != -1)
			this->connection.getServer().removePollEvent(*this, this->pipeIn[1]);
		if (this->pipeOut[0] != -1)
			this->connection.getServer().removePollEvent(*this, this->pipeOut[0]);
	}

	SAFE_CLOSE(this->pipeIn[0]);
	SAFE_CLOSE(this->pipeIn[1]);
	SAFE_CLOSE(this->pipeOut[0]);
	SAFE_CLOSE(this->pipeOut[1]);

	if (childPID != -1)
	{
		::kill(this->childPID, SIGKILL);
		waitpid(this->childPID, NULL, 0);
	}
	
}

static inline std::string	to_env_key(const std::string& header);
static inline std::string	*get_header(const Headers& header, const std::string& key);

void CGIHandler::onCreation()
{
	this->initPaths();
	this->initEnvironment();

	if (!FileSystem::exists(this->scriptFilename))
		throw HTTPException(HTTPCode::NOT_FOUND, "Script file not found");

	if (!FileSystem::isExecutable(this->scriptFilename))
		throw HTTPException(HTTPCode::FORBIDDEN, "File not executable");

	#if HTTP_DEBUG == true
	for (size_t index = 0; index < this->envFlat.size(); ++index)
		std::cout << this->envFlat[index] << "\n";
	#endif

	const char 					*argv[3];
	std::vector<const char *>	envp;

	this->initProcessVariables(argv, envp);

	/*Open pipes for inter process comunication*/
	if (::pipe(this->pipeIn) == -1 || ::pipe(this->pipeOut) == -1)
		throw HTTPException(HTTPCode::INTERNAL_SERVER_ERROR, strerror(errno));

	this->childPID = fork();
	switch (this->childPID)
	{
	case 0: //Child
	{
		::close(this->pipeIn[1]);
		::close(this->pipeOut[0]);

		if (::dup2(this->pipeIn[0], 0) == -1 || ::dup2(this->pipeOut[1], 1) == -1)
		{
			::close(this->pipeIn[0]);
			::close(this->pipeOut[1]);
			_exit(EXIT_FAILURE);
		}

		::close(this->pipeIn[0]);
		::close(this->pipeOut[1]);

		if (::chdir(this->routeResult.basePath.c_str()) == -1)
			_exit(EXIT_FAILURE);
		::execve(argv[0], const_cast<char **>(argv), const_cast<char **>(envp.data()));

		_exit(EXIT_FAILURE);
		break;
	}
	default: //Parent

		::close(this->pipeIn[0]); this->pipeIn[0] = -1;
		::close(this->pipeOut[1]); this->pipeOut[1] = -1;

		if (::fcntl(this->pipeIn[1], F_SETFL, O_NONBLOCK) == -1
				|| ::fcntl(this->pipeOut[0], F_SETFL, O_NONBLOCK) == -1)
			throw HTTPException(HTTPCode::INTERNAL_SERVER_ERROR, strerror(errno));

		TCPServer&	server = this->connection.getServer();
		this->registered = true;
		server.addPollEvent(*this, this->pipeIn[1],  EPOLLERR | EPOLLHUP | EPOLLOUT);
		server.addPollEvent(*this, this->pipeOut[0], EPOLLERR | EPOLLHUP | EPOLLIN);
		break;
	case -1: //Error
		throw HTTPException(HTTPCode::INTERNAL_SERVER_ERROR, strerror(errno));
		break;
	}
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

inline void CGIHandler::initProcessVariables(const char *argv[3], std::vector<const char *>& envp)
{
	const HashMap<std::string, std::string>&	interpreters = this->CGIConfig.interpreters;

	HashMap<std::string, std::string>::const_iterator it = interpreters.find(FileSystem::getExtension(this->scriptName));
	if (it == interpreters.end())
	{
		HashMap<std::string, std::string>::const_iterator it = interpreters.find("default");
		if (it == interpreters.end())
			this->isBinary = true;
		else
			this->interpreter = it->second;
	}
	else
		this->interpreter = it->second;

	argv[0] = this->isBinary ? this->scriptFilename.c_str() : this->interpreter.c_str();
	argv[1] = this->isBinary ? NULL : this->scriptFilename.c_str();
	argv[2] = NULL;

	envp.reserve(this->envFlat.size() + 1);
	for (size_t index = 0; index < this->envFlat.size(); ++index)
		envp.push_back(this->envFlat[index].c_str());
	envp.push_back(NULL);
}

void CGIHandler::onExecute()
{
	if (this->statusCode != HTTPCode::OK)
		throw HTTPException(this->statusCode);
}

void CGIHandler::handleEvent(TCPServer &server, uint32_t events)
{
	(void) server;

	if (events & EPOLLHUP || events & EPOLLRDHUP || events & EPOLLERR )
	{
		if (events & EPOLLERR)
			Logger::ERROR() << "CGI socket errored";
		this->statusCode = HTTPCode::INTERNAL_SERVER_ERROR;
		return ;
	}

	if (events & EPOLLOUT)
	{
		//ssize_t s = ::send(this->pipeIn[1], this->body.getMemoryBuffer())
	}

	if (events & EPOLLIN)
	{
		uint8_t	buffer[4096];
		ssize_t readed = ::recv(this->pipeOut[0], buffer, 4096, 0);
		this->CGIParser.feed(buffer, 4096);
	}
}

static inline std::string to_env_key(const std::string &header)
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
