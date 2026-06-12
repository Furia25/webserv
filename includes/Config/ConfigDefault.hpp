/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigDefault.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/16 19:43:25 by vdurand           #+#    #+#             */
/*   Updated: 2026/06/12 02:26:08 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef _CONFIGDEFAULT_H
# define _CONFIGDEFAULT_H

# define SERV_NAME	"webserv"
# define SERV_VERSION	"1.0.0"

# define CONFIG_MAX_PATH_SIZE 2048

# define CONFIG_MAX_EVENTS	1024

# define CONFIG_READ_SIZE	4096
# define CONFIG_READ_LIMIT	16384

# define CONFIG_CLOSING_TIMEOUT	5
# define CONFIG_ABSOLUTE_TIMEOUT	6

# define CONFIG_BODY_SIZE	10485760
# define CONFIG_TICK_INTERVAL	5

#endif // _CONFIGDEFAULT_H