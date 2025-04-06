/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommonFunctions.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/06 23:23:09 by mspasic           #+#    #+#             */
/*   Updated: 2025/04/06 23:23:34 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CommonFunctions.hpp"


void ftMemset(void *dest, std::size_t count){
    unsigned char *p = (unsigned char*)dest;
    for (std::size_t i = 0; i < count; i++){
        p[i] = 0;
    }
}
