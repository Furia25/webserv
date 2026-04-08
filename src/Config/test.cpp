/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/08 09:48:28 by vdurand           #+#    #+#             */
/*   Updated: 2026/04/08 10:52:34 by vdurand          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "Variant.hpp"

# include <iostream>

int main(void)
{
	Variant test(5LL);
	test = std::string("fdsfdsfd");
	test = 10LL;
	test = 5LL;
	test = std::string("fdsfdsfdhgjgfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhg");
		test = std::string("fdsfdsfd");
	test = 10LL;
	test = 5LL;
	test = std::string("fdsfdsfdhgjgfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhg");
		test = std::string("fdsfdsfd");
	test = 10LL;
	test = 5LL;
	test = std::string("fdsfdsfdhgjgfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhg");
		test = std::string("fdsfdsfd");
	test = 10LL;
	test = 5LL;
	test = std::string("fdsfdsfdhgjgfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhg");
		test = std::string("fdsfdsfd");
	test = 10LL;
	test = 5LL;
	test = std::string("fdsfdsfdhgjgfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhg");
		test = std::string("fdsfdsfd");
	test = 10LL;
	test = 5LL;
	test = std::string("fdsfdsfdhgjgfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhg");
		test = std::string("fdsfdsfd");
	test = 10LL;
	test = 5LL;
	test = std::string("fdsfdsfdhgjgfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhggfdgfdgfdgfdgfdhg");
	std::cout << test.as<std::string &>();
}