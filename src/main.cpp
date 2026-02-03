/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/03 14:24:55 by vdurand           #+#    #+#             */
/*   Updated: 2026/02/03 16:38:44 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "MIME.hpp"
#include <iostream>
#include <cstdlib>

#define PRINT(...) #_VA_LIST;

int	main(void)
{
	PRINT("Test", "test", "test");
}
