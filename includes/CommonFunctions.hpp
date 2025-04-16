// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<CommonFunctions.hpp>> -- <<Aida, Ilmari, Milica>>

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
