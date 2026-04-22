
#ifndef _HTTPTYPES_H
# define _HTTPTYPES_H

# include <sstream>
# include <string>
# include <stdexcept>
# include <iostream>

# include "EnumClass.hpp"
# include "MIME.hpp"

#define PROTOCOL "HTTP"
#define _DEFAULT_MAX_BODY_SIZE_ 10485760
#define _DEFAULT_MAX_PATH_SIZE_ 2048

ENUM_CLASS(HandlerType, (STATIC, UPLOAD, CGI, REDIRECT, STATUS), ENUM_BASIC);

# define _METHODS_ (GET, POST, DELETE, HEAD, PUT)
ENUM_CLASS(Method, _METHODS_, ENUM_BASIC, ENUM_LITERALS(_METHODS_, ENUM_BASIC, ENUM_BASIC););
# undef _METHODS_

#define _STATUS_CODES_ \
(\
	/*	2xx Success	*/ \
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
# define X_STRING(tuple, ...)	_M_TUPLE_ELEM_1 tuple __VA_ARGS__
ENUM_CLASS(HTTPCode, _STATUS_CODES_, X, ENUM_LITERALS(_STATUS_CODES_, X_STRING_CODE, X_STRING); static bool is_error(HTTPCode code) { return static_cast<int>(code) >= 400; };);
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
	HTTPException(HTTPCode code) : code(code) {}
	virtual ~HTTPException() throw() {};

	virtual const char* what() const throw() { return message.c_str(); };
	HTTPCode			getStatusCode() const { return this->code; };
	const std::string&	getSummary() const { return this->summary; };
private:
	const HTTPCode		code;
	const std::string	summary;
	std::string			message;
};

#endif // _HTTPTYPES_H