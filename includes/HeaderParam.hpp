/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HeaderParam.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 17:41:18 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/13 13:30:26 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _HEADERPARAM_H
# define _HEADERPARAM_H

# include "EnumClass.hpp"

#define _HEADER_PARAM (\
	(HOST, host), \
	(CONTENT_LENGTH, content-length) \
) \


# define X(tuple, ...)	M_TUPLE_ELEMENT(tuple,0) __VA_ARGS__
# define X_STRING(tuple, ...)	M_TUPLE_ELEMENT(tuple, 1) __VA_ARGS__
ENUM_CLASS(HeaderParam, _HEADER_PARAM, X, ENUM_LITERALS(_HEADER_PARAM, X, X_STRING););
# undef X
# undef X_STRING


#endif // _HEADERPARAM_H