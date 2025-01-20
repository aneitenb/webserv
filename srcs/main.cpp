/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aneitenb <aneitenb@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/20 12:50:45 by aneitenb          #+#    #+#             */
/*   Updated: 2025/01/20 15:49:14 by aneitenb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Webserv.hpp"
#include "../includes/ConfigFile.hpp"

int main (int argc, char **argv)
{
	if (argc == 1 || argc == 2)
	{
		try
		{
			ConfigFile	parser;

			parser.initializeConfFile(argc, argv);
			//set up servers
			//run servers-- server manager class?
		}
		catch (std::exception &e) 
		{
			std::cerr << e.what() << std::endl;
			return (1);
		}
	}
	else
	{
		std::cout << "Webserv: Wrong amount of parameters. Cannot initialize server." << std::endl;
		return (1);
	}
	return (0);
}