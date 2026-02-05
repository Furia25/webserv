/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 14:24:55 by vdurand           #+#    #+#             */
/*   Updated: 2026/02/05 18:38:23 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "MIME.hpp"
#include <iostream>
#include <cstdlib>

int	main(void)
{
	std::string	test("text/plain");
	std::cout << TEST::toString(MIME::from(test)) << std::endl;
}
