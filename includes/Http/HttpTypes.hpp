/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpTypes.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 18:24:24 by antbonin          #+#    #+#             */
/*   Updated: 2026/04/15 18:25:22 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _HTTPTYPES_H
# define _HTTPTYPES_H

#include <string>

#define PROTOCOL "HTTP"
#define _MAX_BODY_SIZE_ 10485760
#define _MAX_PATH_SIZE_ 2048

enum	Method
{
	GET,
	POST,
	DELETE,
	UNKNOWN,
};

static const std::string allowed_method[] = {"GET","POST","DELETE"};

#endif // _HTTPTYPES_H