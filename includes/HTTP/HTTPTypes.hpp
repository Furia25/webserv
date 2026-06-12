/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HTTPTypes.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 14:56:01 by antbonin          #+#    #+#             */
/*   Updated: 2026/06/12 18:01:09 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _HTTPTYPES_H
# define _HTTPTYPES_H

# include <sstream>
# include <string>
# include <stdexcept>
# include <iostream>
# include <sstream>

# include "EnumClass.hpp"
# include "MIME.hpp"
# include "Utils/HashMap.hpp"
# include "Utils/Hash.hpp"
# include "Config/ConfigDefault.hpp"
# include "Utils/IntegerUtils.hpp"

# define HTTP_DEBUG	false

# define CGI_VERSION		"CGI/1.1"
# define HTTP_VERSION		"HTTP/1.1"

# define HEADER_HOST 				"host"
# define HEADER_COOKIE				"cookie"
# define HEADER_CONNECTION			"connection"
# define HEADER_CONTENT_TYPE		"content-type"
# define HEADER_CONTENT_LENGTH 		"content-length"
# define HEADER_TRANSFER_ENCODING	"transfer-encoding"

# define HTTP_NEWLINE	"\r\n"

# define _HANDLERTYPES_	(STATIC, UPLOAD, CGI, REDIRECT, STATUS)
ENUM_CLASS(HandlerType, _HANDLERTYPES_, ENUM_BASIC,
	ENUM_LITERALS(_HANDLERTYPES_, ENUM_BASIC, ENUM_BASIC);
	public: HandlerType() : _t(_M_TUPLE_ELEM_0 _HANDLERTYPES_) {};
);
# undef _HANDLERTYPES_

# define _METHODS_ (GET, POST, DELETE, HEAD, UNKNOWN)
ENUM_CLASS(Method, _METHODS_, ENUM_BASIC, ENUM_LITERALS(_METHODS_, ENUM_BASIC, ENUM_BASIC); public: Method() : _t(UNKNOWN) {});
# undef _METHODS_

#define _STATUS_CODES_ \
(\
	/*	2xx Success	*/ \
	(CONTINUE,					100, Continue), \
	(OK,						200, OK), \
	(CREATED,					201, Created), \
	(ACCEPTED,					202, Accepted), \
	(NO_CONTENT,				204, No Content), \
	(PARTIAL_CONTENT,			206, Partial Content), \
	/*	3xx Redirection	*/ \
	(MOVED_PERMANENTLY,			301, Moved Permanently), \
	(FOUND,						302, Found), \
	(NOT_MODIFIED,				304, Not Modified), \
	(TEMPORARY_REDIRECT,		307, Temporary Redirect), \
	(PERMANENT_REDIRECT,		308, Permanent Redirect), \
	/*	4xx Client Error	*/ \
	(BAD_REQUEST,				400, Bad Request), \
	(UNAUTHORIZED,				401, Unauthorized), \
	(PAYMENT_REQUIRED,			402, Payment Required), \
	(FORBIDDEN,					403, Forbidden), \
	(NOT_FOUND,					404, Not Found), \
	(METHOD_NOT_ALLOWED,		405, Method Not Allowed), \
	(NOT_ACCEPTABLE,			406, Not Acceptable), \
	(REQUEST_TIMEOUT,			408, Request Timeout), \
	(CONFLICT,					409, Conflict), \
	(GONE,						410, Gone), \
	(LENGTH_REQUIRED,			411, Length Required), \
	(PRECONDITION_FAILED,		412, Precondition Failed), \
	(PAYLOAD_TOO_LARGE,			413, Payload Too Large), \
	(URI_TOO_LONG,				414, URI Too Long), \
	(UNSUPPORTED_MEDIA,			415, Unsupported Media Type), \
	(EXPECTATION_FAILED,		417, Expectation Failed), \
	(TOO_MANY_REQUESTS,			429, Too Many Requests), \
	(HEADER_FIELDS_TOO_LARGE,	431, Request Header Fields Too Large), \
	/*	5xx Server Error	*/ \
	(INTERNAL_SERVER_ERROR,		500, Internal Server Error), \
	(NOT_IMPLEMENTED,			501, Not Implemented), \
	(BAD_GATEWAY,				502, Bad Gateway), \
	(SERVICE_UNAVAILABLE,		503, Service Unavailable), \
	(GATEWAY_TIMEOUT,			504, Gateway Timeout), \
	(HTTP_VERSION_NOT_SUPPORTED,505, HTTP Version Not Supported) \
)

# define X(tuple, ...)	_M_TUPLE_ELEM_0 tuple = _M_TUPLE_ELEM_1 tuple __VA_ARGS__
# define X_STRING_CODE(tuple, ...)	_M_TUPLE_ELEM_0 tuple __VA_ARGS__
# define X_STRING(tuple, ...)	_M_TUPLE_ELEM_2 tuple __VA_ARGS__
ENUM_CLASS(HTTPCode, _STATUS_CODES_, X,
	ENUM_LITERALS(_STATUS_CODES_, X_STRING_CODE, X_STRING);
	public: HTTPCode() : _t(NOT_FOUND) {};
	static bool is_error(HTTPCode code) { return static_cast<int>(code) >= 400; };
	static HTTPCode	fromLiteral(const std::string& str)
	{
		HTTPCode code = HTTPCode::NOT_FOUND;
		try { code = HTTPCode::from(str); }
		catch (const std::domain_error&)
		{
			size_t integer = IntegerUtils::strtoul_safe(str.c_str(), 10);
			code = static_cast<HTTPCode::E>(integer);
		}
		return code;
	};
);
# undef X
# undef X_STRING_CODE
# undef X_STRING
# undef _STATUS_CODES_

class HTTPException : public std::exception
{
public:
	HTTPException(HTTPCode code, const std::string& summary) : code(code), summary(summary)
	{
		std::stringstream	ss;
		ss << static_cast<int>(code) << " " << HTTPCode::toString(code);
		message = ss.str();
		if (!summary.empty())
			message += " (" + summary + ")";
	}

	HTTPException(HTTPCode code) : code(code)
	{
		std::stringstream	ss;
		ss << static_cast<int>(code) << " " << HTTPCode::toString(code);
		message = ss.str();
	}

	virtual ~HTTPException() throw() {};

	virtual const char* what() const throw() { return message.c_str(); };
	HTTPCode			getStatusCode() const { return this->code; };
	const std::string&	getSummary() const { return this->summary; };
private:
	const HTTPCode		code;
	const std::string	summary;
	std::string			message;
};

template <>
struct Hash<HTTPCode>
{
	size_t operator()(HTTPCode key) const { return hash_int(key); }
};

typedef HashMap<std::string, std::string> Cookies;
typedef HashMap<std::string, std::string> Headers;

namespace Cookie
{
	# define _SAMESITE_ (LAX, STRICT, NONE)
	ENUM_CLASS(SameSite, _SAMESITE_, ENUM_BASIC, ENUM_LITERALS(_SAMESITE_, ENUM_BASIC, ENUM_BASIC); public: SameSite() : _t(LAX) {});
	# undef _SAMESITE_
} // namespace Cookie

#endif // _HTTPTYPES_H