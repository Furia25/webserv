/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   EnumClass.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 16:05:08 by vdurand           #+#    #+#             */
/*   Updated: 2026/02/03 16:41:23 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _TEST_H
# define _TEST_H

#define ENUM_CLASS_ENUM(val) val,
#define ENUM_CLASS_CASE(val) case val: return #val;
#define ENUM_CLASS_STRING(val) #val,

#define ENUM_CLASS_FULL(name, XMACRO, MACRO_STRING)\
struct name {\
	enum Enum {\
		XMACRO(ENUM_CLASS_ENUM)\
		MAX \
	};\
	static const std::string&	toString(Enum e)\
	{\
		static const std::string values[] = {\
			XMACRO_STRING(ENUM_CLASS_STRING)\
		};\
		int index = static_cast<int>(e);\
		if (index < 0 || index >= Enum::MAX)\
		return unknown();\
		return values[index];\
	}\
	private:\
	static const std::string& unknown() {\
		static const std::string unk("Unknown");\
		return unk;\
	} \
};

#define ENUM_CLASS(name, XMACRO)

#endif // _TEST_H