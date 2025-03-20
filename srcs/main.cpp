/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: aneitenb <aneitenb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/20 12:50:45 by aneitenb          #+#    #+#             */
/*   Updated: 2025/03/20 16:32:20 by aneitenb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"
#include "config/ConfigFile.hpp"

int main(int argc, char* argv[]) 
{
    std::string configPath = "";
    
    if (argc > 1) {
        configPath = argv[1];
    }
    try {
        Server server(configPath);
        
        server.run();
    } 
	catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}