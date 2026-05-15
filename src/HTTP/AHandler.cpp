/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   AHandler.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/27 15:57:58 by vdurand           #+#    #+#             */
/*   Updated: 2026/05/15 05:31:36 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "HTTP/AHandler.hpp"
# include "Utils/FileSystem.hpp"
# include "HTTP/Response.hpp"
# include "Utils/IntegerUtils.hpp"

bool AHandler::execute()
{
	if (this->finished)
		return false;
	if (this->errored)
		this->handleError();
	try
	{
		if (this->first)
		{
			this->onCreation();
			this->first = false;
		}
		this->onExecute();
	}
	catch (const HTTPException& http_exception)
	{
		this->statusCode = http_exception.getStatusCode();
		this->errored = true;
	}
	catch (...)
	{
		this->statusCode = HTTPCode::INTERNAL_SERVER_ERROR;
		this->errored = true;
	}

	return !this->finished;
}

void AHandler::sendFullDefaultError()
{
	const std::string codeStr = IntegerUtils::itoa(this->statusCode);
	const char *codeMsg = HTTPCode::toString(this->statusCode);

	std::string body =
		"<!DOCTYPE html>\n"
		"<html lang=\"en\">\n"
		"<head>\n"
		"  <meta charset=\"UTF-8\">\n"
		"  <title>" + codeStr + " " + codeMsg + "</title>\n"
		"  <style>\n"
		"    body { font-family: monospace; background: #1a1a1a; color: #d4d4d4;"
		"           margin: 0; padding: 60px 40px; }\n"
		"    h1   { font-size: 2.8em; font-weight: 400; color: #e0e0e0; margin: 0 0 8px; }\n"
		"    .sep { width: 40px; height: 2px; background: #555; margin: 16px 0 24px; }\n"
		"    p    { font-size: 0.9em; color: #888; line-height: 1.7; }\n"
		"    .url { color: #c0a0f0; }\n"
		"    hr   { border: none; border-top: 1px solid #2a2a2a; margin-top: 40px; }\n"
		"    footer { font-size: 0.75em; color: #555; margin-top: 12px; }\n"
		"  </style>\n"
		"</head>\n"
		"<body>\n"
		"  <h1>" + codeStr + " " + codeMsg + "</h1>\n"
		"  <div class=\"sep\"></div>\n"
		"  <p>The requested URL <span class=\"url\">" + this->request.path + "</span>"
		" was not found on this server.</p>\n"
		"  <hr>\n"
		"  <footer>" SERV_NAME "/" SERV_VERSION "</footer>\n"
		"</body>\n"
		"</html>\n";

	response.sendContentType(MIME::html)
		.sendContentLength(body.size())
		.sendBody(body)
		.sendEnd();
}

void AHandler::handleError()
{
	switch (this->state)
	{
	case INIT:
	{
		if (this->routeResult.host->error_fallbacks.contain(statusCode))
		{
			std::string errorFile = this->routeResult.host->error_fallbacks.at(statusCode);
			physicalPath = this->routeResult.host->root + "/" + errorFile;
		}
		if (!physicalPath.empty() && FileSystem::exists(physicalPath) && FileSystem::isFile(physicalPath))
			this->state = SEND_HEADERS;
		else
			this->state = SEND_DEFAULT_ERROR;
		break;
	}

	case SEND_HEADERS:
	{
		try  { this->fileReader.open(physicalPath); }
		catch (...) { this->state = SEND_DEFAULT_ERROR;}

		size_t	fileSize = this->fileReader.getFileSize();
		MIME 	mime_type = MIME::from_extension(FileSystem::getExtension(physicalPath));
		response.sendStatusLine(statusCode)
				.sendDefaults(this->request, *this->routeResult.route)
				.sendContentType(mime_type)
				.sendContentLength(fileSize);
		if (request.method == Method::HEAD)
		{
			this->state = FINISHED;
			this->fileReader.close();
		}
		else
			this->state = SEND_BODY;
		break;
	}

	case SEND_BODY:
	{
		std::vector<uint8_t> buffer;
		size_t bytes_read = this->fileReader.readChunk(buffer, 8192);

		if (bytes_read > 0)
			response.sendBody(buffer.data(), bytes_read);

		if (this->fileReader.hasFinished())
		{
			this->fileReader.close();
			this->state = FINISHED;
		}
		break;
	}

	case SEND_DEFAULT_ERROR:
	{
		response.sendStatusLine(statusCode)
				.sendDefaults(this->request, *this->routeResult.route);
		if (request.method != Method::HEAD)
			this->sendFullDefaultError();
		this->state = FINISHED;
		break;
	}

	case FINISHED:
		response.sendEnd();
		this->setFinished();
		break;
	}
}
