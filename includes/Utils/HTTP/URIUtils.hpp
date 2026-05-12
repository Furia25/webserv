/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   URIUtils.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antoine <antoine@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/13 00:22:32 by antoine           #+#    #+#             */
/*   Updated: 2026/05/13 00:31:42 by antoine          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef URIUTILS_HPP
# define URIUTILS_HPP

# include <iostream>
# include <vector>

std::string normalizePath(const std::string& path);
std::string	decodeURI(const std::string& input);

# endif