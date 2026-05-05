/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DirectoryHandler.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: antbonin <antbonin@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/05/05 16:59:17 by antbonin          #+#    #+#             */
/*   Updated: 2026/05/05 17:58:24 by antbonin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _DIRECTORYHANDLER_H
# define _DIRECTORYHANDLER_H

# include <iostream>

class DirectoryHandler
{
public:
	size_t	getDirectorySize(const std::string& path) const;
private:
};

#endif // _DIRECTORYHANDLER_H
