/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/15 18:10:23 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/15 18:11:00 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _CONFIG_H
# define _CONFIG_H

# include "ConfigStructs.hpp"

class Config
{
public:
	Config();
	Config(const Config& other);
	~Config();
	Config& operator=(const Config& other);
protected:
private:
};

#endif // _CONFIG_H