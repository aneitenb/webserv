// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<message.cpp>> -- <<Aida, Ilmari, Milica>>

#include <iostream>

#include "utils/message.hpp"

#define SGR_RESET	"\x1b[m"

#define setaf(color)	"\x1b[1;38;5;" << static_cast<i32>(color) << "m"

static inline void	message(const std::stringstream &msg) {
	std::cerr << msg.str();
}

void	error([[maybe_unused]] const std::stringstream &msg, [[maybe_unused]] const u8 color) {
#if LOG_LEVEL >= LOG_ERROR
	std::cerr << setaf(color);
	message(msg);
	std::cerr << SGR_RESET << '\n';
#endif
}

void	warn([[maybe_unused]] const std::stringstream &msg, [[maybe_unused]] const u8 color) {
#if LOG_LEVEL >= LOG_WARN
	std::cerr << setaf(color);
	message(msg);
	std::cerr << SGR_RESET << '\n';
#endif
}

void	info([[maybe_unused]] const std::stringstream &msg, [[maybe_unused]] const u8 color) {
#if LOG_LEVEL >= LOG_INFO
	std::cerr << setaf(color);
	message(msg);
	std::cerr << SGR_RESET << '\n';
#endif
}

void	debug([[maybe_unused]] const std::stringstream &msg, [[maybe_unused]] const u8 color) {
#if LOG_LEVEL >= LOG_DEBUG
	std::cerr << setaf(color);
	message(msg);
	std::cerr << SGR_RESET << '\n';
#endif
}
