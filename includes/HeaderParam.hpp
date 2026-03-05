/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HeaderParam.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/03/05 17:41:18 by vdurand           #+#    #+#             */
/*   Updated: 2026/03/05 17:42:34 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _HEADERPARAM_H
# define _HEADERPARAM_H

# include <string>
# include <ostream>

# include "macrosplosion.hpp"
# include "EnumClass.hpp"

#define _HEADER_PARAM (\
	(HOST, host), \
	(CONTENT_LENGTH, content-length) \
) \


# define X(tuple, ...)	M_TUPLE_ELEMENT(tuple,0),
# define X_STRING(tuple, ...)	M_TUPLE_ELEMENT(tuple, 1)
ENUM_CLASS(HEADER_PARAM, _HEADER_PARAM, X, ENUM_LITERALS(_HEADER_PARAM, X_STRING, ~); );
# undef X
# undef X_STRING


#endif // _HEADERPARAM_H