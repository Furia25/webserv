/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpTypes.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 22:08:40 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/15 22:44:41 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _HTTPTYPES_H
# define _HTTPTYPES_H

# include <string>
# include <stdexcept>

# include "EnumClass.hpp"
# include "MIME.hpp"

#define PROTOCOL "HTTP"
#define _MAX_BODY_SIZE_ 10485760
#define _MAX_PATH_SIZE_ 2048

ENUM_CLASS(HandlerType, (STATIC, UPLOAD, CGI, REDIRECT), ENUM_BASIC);

# define _METHODS_ (GET, POST, DELETE, HEAD, PUT, UNKNOWN)
ENUM_CLASS(Method, _METHODS_, ENUM_BASIC, ENUM_LITERALS(_METHODS_, ENUM_BASIC, ENUM_BASIC););
# undef _METHODS_

#define _STATUS_CODES_ \
(\
	(OK,					200, OK), \
	(CREATED,				201, Created), \
	(ACCEPTED,				202, Accepted), \
	(NO_CONTENT,			204, No Content), \
	(MOVED_PERMANENTLY,		301, Moved Permanently),  \
	(MOVED_TEMPORARILY,		302, Moved Temporarily),  \
	(NOT_MODIFIED,			304, Not Modified), \
	(BAD_REQUEST,			400, Bad Request), \
	(UNAUTHORIZED,			401, Unauthorized),  \
	(FORBIDDEN,				403, Forbidden), \
	(NOT_FOUND,				404, Not Found), \
	(INTERNAL_SERVER_ERROR,	500, Internal Server Error), \
	(NOT_IMPLEMENTED,		501, Not Implemented), \
	(BAD_GATEWAY,			502, Bad Gateway),  \
	(SERVICE_UNAVAILABLE,	503, Service Unavailable) \
)

# define X(tuple, ...)	_M_TUPLE_ELEM_0 tuple = _M_TUPLE_ELEM_1 tuple __VA_ARGS__
# define X_STRING_CODE(tuple, ...)	_M_TUPLE_ELEM_0 tuple __VA_ARGS__
# define X_STRING(tuple, ...)	_M_TUPLE_ELEM_1 tuple __VA_ARGS__
ENUM_CLASS(HTTPCode, _STATUS_CODES_, X, ENUM_LITERALS(_STATUS_CODES_, X_STRING_CODE, X_STRING););
# undef X
# undef X_STRING_CODE
# undef X_STRING
# undef _STATUS_CODES_


#endif // _HTTPTYPES_H