/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GenerateUniqueFilename.hpp                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/05 17:25:01 by antbonin          #+#    #+#             */
/*   Updated: 2026/06/05 17:29:27 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef _GENERATEUNIQUEFILENAME_H
# define _GENERATEUNIQUEFILENAME_H

#include <sys/time.h>
#include <sstream>
#include <cstdlib>
#include <string>

std::string	GenerateUniqueFilename(const std::string& baseName);

#endif // _GENERATEUNIQUEFILENAME_H