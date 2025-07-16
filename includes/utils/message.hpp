// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<message.hpp>> -- <<Aida, Ilmari, Milica>>

#pragma once

#include <sstream>

#include "defs.hpp"

#define LOG_NONE	0
#define LOG_ERROR	1
#define LOG_WARN	2
#define LOG_INFO	3
#define LOG_DEBUG	4

#ifndef LOG_LEVEL
# define LOG_LEVEL	LOG_INFO
#endif

#ifndef COLOR_ERROR
# define COLOR_ERROR	196
#endif

#ifndef COLOR_WARN
# define COLOR_WARN		202
#endif

#ifndef COLOR_INFO
# define COLOR_INFO		99
#endif

#ifndef COLOR_DEBUG
# define COLOR_DEBUG	85
#endif

void	error(const std::stringstream &msg, const u8 color = COLOR_ERROR);
void	warn(const std::stringstream &msg, const u8 color = COLOR_WARN);
void	info(const std::stringstream &msg, const u8 color = COLOR_INFO);
void	debug(const std::stringstream &msg, const u8 color = COLOR_DEBUG);

inline void	error(const std::string &msg, const u8 color = COLOR_ERROR) { return error(std::stringstream(msg), color); }
inline void	warn(const std::string &msg, const u8 color = COLOR_WARN) { return warn(std::stringstream(msg), color); }
inline void	info(const std::string &msg, const u8 color = COLOR_INFO) { return info(std::stringstream(msg), color); }
inline void	debug(const std::string &msg, const u8 color = COLOR_DEBUG) { return debug(std::stringstream(msg), color); }

#define Error(msg)	(error(std::stringstream("") << msg))
#define Warn(msg)	(warn(std::stringstream("") << msg))
#define Info(msg)	(info(std::stringstream("") << msg))
#define Debug(msg)	(debug(std::stringstream("") << msg))
