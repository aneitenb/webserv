/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommonFunctions.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mspasic <mspasic@student.hive.fi>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/06 23:21:26 by mspasic           #+#    #+#             */
/*   Updated: 2025/04/16 17:57:45 by ivalimak         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <regex>
#include <dirent.h>
#include <filesystem>

//STREAM
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

//OTHER
#include <vector>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <sys/stat.h>
#include <set>
#include <unistd.h>

#include <exception>

void ftMemset(void *dest, std::size_t count);
