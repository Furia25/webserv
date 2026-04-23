/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test_config.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/23 03:03:24 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/23 03:05:24 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "Config/Config.hpp"

int main(void)
{
	try {Config::AppConfig config("config.toml");}
	catch (const std::exception& e)
	{
		Logger::FATAL() << "Unable to recover from fatal error : \"" << e.what() << "\"\n	errno: " << strerror(errno);
	}
}
