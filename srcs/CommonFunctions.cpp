/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommonFunctions.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/06 23:23:09 by mspasic           #+#    #+#             */
/*   Updated: 2025/04/08 20:12:30 by mspasic          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CommonFunctions.hpp"
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>

void check_fds() {
    DIR *dir = opendir("/proc/self/fd");
    if (dir == NULL) {
        perror("opendir");
        return;
    }

    printf("Open file descriptors:\n");
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] != '.')
            printf("FD: %s\n", entry->d_name);
    }
    closedir(dir);
}

void ftMemset(void *dest, std::size_t count){
    unsigned char *p = (unsigned char*)dest;
    for (std::size_t i = 0; i < count; i++){
        p[i] = 0;
    }
}
