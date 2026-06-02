/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIHandler.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/22 04:19:31 by vdurand           #+#    #+#             */
/*   Updated: 2026/06/02 19:28:31 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Utils/FileSystem.hpp"
#include "HTTP/Handler/CGIHandler.hpp"
#include "Server/TCPServer.hpp"
#include <sys/wait.h>

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

	SAFE_CLOSE(this->bodyFD);

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

	#if HTTP_DEBUG == true
	for (size_t index = 0; index < this->envFlat.size(); ++index)
		std::cout << this->envFlat[index] << "\n";
	#endif

	if (!FileSystem::exists(this->scriptFilename))
		throw HTTPException(HTTPCode::NOT_FOUND, "Script file not found");

	if (!FileSystem::isExecutable(this->scriptFilename))
		throw HTTPException(HTTPCode::FORBIDDEN, "File not executable");

	const char 					*argv[3];
	std::vector<const char *>	envp;

	this->initProcessVariables(argv, envp);

	if (this->body.getIsStreaming())
	{
		this->bodyFD = open(this->body.getFilePath().c_str(), 0);
		if (this->bodyFD == -1)
			throw HTTPException(HTTPCode::INTERNAL_SERVER_ERROR, strerror(errno));
	}

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
	{
		::close(this->pipeIn[0]); this->pipeIn[0] = -1;
		::close(this->pipeOut[1]); this->pipeOut[1] = -1;

		if (::fcntl(this->pipeIn[1], F_SETFL, O_NONBLOCK) == -1
				|| ::fcntl(this->pipeOut[0], F_SETFL, O_NONBLOCK) == -1)
			throw HTTPException(HTTPCode::INTERNAL_SERVER_ERROR, strerror(errno));

		TCPServer& server = this->connection.getServer();
		this->registered = true;
		server.addPollEvent(*this, this->pipeIn[1],  EPOLLERR | EPOLLHUP | EPOLLOUT);
		server.addPollEvent(*this, this->pipeOut[0], EPOLLERR | EPOLLHUP | EPOLLIN);
	}
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

	std::string firstSegment;

	if (remainder.size() > 1)
		firstSegment = remainder.substr(1, slash == std::string::npos ? std::string::npos : slash - 1);

	bool isExplicitScript = false;

	size_t dot = firstSegment.rfind('.');
	if (dot != std::string::npos)
	{
		std::string ext = firstSegment.substr(dot + 1);
		isExplicitScript = this->CGIConfig.interpreters.contain(ext) > 0;
	}

	if (isExplicitScript)
	{
		this->scriptName = firstSegment;
		if (slash != std::string::npos)
			this->pathInfo = remainder.substr(slash);
	}
	else if (!this->CGIConfig.default_bin.empty())
	{
		this->scriptName = this->CGIConfig.default_bin;
		this->pathInfo   = remainder;
	}

	this->scriptFilename = this->routeResult.basePath + "/" + this->scriptName;
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

	env.insert("SCRIPT_NAME", this->CGIConfig.path + "/" + this->scriptName);

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

	argv[0] = this->isBinary ? this->scriptName.c_str() : this->interpreter.c_str();
	argv[1] = this->isBinary ? NULL : this->scriptName.c_str();
	argv[2] = NULL;

	envp.reserve(this->envFlat.size() + 1);
	for (size_t index = 0; index < this->envFlat.size(); ++index)
		envp.push_back(this->envFlat[index].c_str());
	envp.push_back(NULL);
}

void CGIHandler::onExecute()
{
	if (this->statusCode != HTTPCode::OK)
	{
		if (this->response.hasStatus())
		{
			this->response
				.sendDefaults(this->request, this->routeResult.route, true)
				.sendEnd();
			this->setFinished();
			return ;
		}
		else
			throw HTTPException(this->statusCode);
	}

	if (this->isCGICompleted)
	{
		this->response.sendEnd();
		this->setFinished();
	}
}

void CGIHandler::sendHeaders()
{
	if (this->response.hasStatus())
		return ;
	Headers::iterator it = this->headers.find("status");
	if (it != this->headers.end())
	{
		this->response.sendStatusLine(it->second);
		this->headers.erase(it);
	}
	else
		this->response.sendStatusLine(HTTPCode::OK);
	this->response.sendDefaults(this->request, &this->CGIConfig);
	this->response.sendHeaders(this->headers);
	this->response.setChunked();
	if (this->readedSize > 0)
		this->response.sendChunk(this->buffer, this->readedSize);
	this->isCGICompleted = true;
}

void CGIHandler::handleEvent(TCPServer& server, uint32_t events)
{
	if (this->statusCode != HTTPCode::OK)
		return ;

	if (events & EPOLLHUP || events & EPOLLRDHUP || events & EPOLLERR)
	{
		if (events & EPOLLERR)
		{
			Logger::ERROR() << "CGI pipe errored";
			this->statusCode = HTTPCode::INTERNAL_SERVER_ERROR;
			return ;
		}

		this->sendHeaders();
		return ;
	}
	if (this->isHeaderParsed)
		this->sendHeaders();

	if (events & EPOLLOUT)
		this->handleWrite(server);

	if (events & EPOLLIN)
	{
		if (!this->isHeaderParsed)
			this->parseCGIHeader();
		else
			this->proxyBody();
	}
}

void CGIHandler::handleWrite(TCPServer& server)
{
	if (this->bodyFD == -1)
	{
		size_t  size = this->body.getSize();

		ssize_t writed = ::write(this->pipeIn[1],
			this->body.getMemoryBuffer().data() + this->writedSize,
			size - this->writedSize);

		if (writed == -1)
		{
			this->statusCode = HTTPCode::INTERNAL_SERVER_ERROR;
			return ;
		}

		if (writed == 0 || this->writedSize + writed >= size)
		{
			server.removePollEvent(*this, this->pipeIn[1]);
			return ;
		}

		this->writedSize += writed;
		return ;
	}

	loff_t offset = static_cast<loff_t>(this->writedSize);

	ssize_t writed = splice(
		this->bodyFD, &offset,
		this->pipeIn[1], NULL,
		65536,
		SPLICE_F_MOVE
	);

	if (writed == -1)
	{
		this->statusCode = HTTPCode::INTERNAL_SERVER_ERROR;
		return ;
	}

	if (writed == 0)
	{
		server.removePollEvent(*this, this->pipeIn[1]);
		return ;
	}

	this->writedSize += writed;
}

void CGIHandler::parseCGIHeader()
{
	ssize_t readed = ::read(this->pipeOut[0],
		this->buffer + this->readedSize,
		CGI_BUFFER_SIZE - this->readedSize);

	if (readed <= 0)
	{
		this->statusCode = HTTPCode::INTERNAL_SERVER_ERROR;
		return;
	}

	this->readedSize += readed;

	size_t	total_parsed = 0;
	size_t remaining;

	while ((remaining = this->readedSize - total_parsed) > 0)
	{
		if (this->buffer[total_parsed] == '\n' || (remaining >= 2 && this->buffer[total_parsed] == '\r' && this->buffer[total_parsed + 1] == '\n'))
		{
			total_parsed += (this->buffer[total_parsed] == '\r') ? 2 : 1;
			this->isHeaderParsed = true;
			break ;
		}

		std::string key;
		std::string value;
		size_t parsed = 0;

		try
		{
			parsed = HeaderParser::tryParseHeaderLine(
				this->buffer + total_parsed,
				remaining, key, value);
		}
		catch (const std::exception& e)
		{
			this->statusCode = HTTPCode::INTERNAL_SERVER_ERROR;
			return;
		}

		if (parsed == 0)
			break;

		this->firstHeader = true;
		this->headers.insert(key, value);
		total_parsed += parsed;
	}

	std::memmove(this->buffer,
		this->buffer + total_parsed,
		this->readedSize - total_parsed);
	this->readedSize -= total_parsed;

	if (this->readedSize >= CGI_BUFFER_SIZE)
	{
		this->statusCode = HTTPCode::INTERNAL_SERVER_ERROR;
		return;
	}
}

void CGIHandler::proxyBody()
{
	ssize_t readed = ::read(this->pipeOut[0], this->buffer, CGI_BUFFER_SIZE);

	if (readed == -1)
	{
		this->statusCode = HTTPCode::INTERNAL_SERVER_ERROR;
		return;
	}

	if (readed == 0)
	{
		this->isCGICompleted = true;
		return ;
	}

	this->readedSize += readed;
	this->response.sendChunk(this->buffer, readed);
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

void cgi_timeout_callback(Alarm<CGIHandler *> &alarm, CGIHandler *handler)
{
	(void) alarm;
	handler->statusCode = HTTPCode::GATEWAY_TIMEOUT;
	Logger::ERROR() << "CGI timeout on path " << handler->request.path;
}
