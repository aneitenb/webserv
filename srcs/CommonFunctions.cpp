// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<CommonFunctions.cpp>> -- <<Aida, Ilmari, Milica>>

#include <unistd.h>

#include "utils/message.hpp"
#include "CommonFunctions.hpp"

#undef Info
#define Info(msg, color)	(info(std::stringstream("") << msg, color))

void	printBody(const std::string &contentType, const std::string &body, const u8 color) {
	Info("\t\tContent-Type: " << contentType << " {", color);
	std::cerr << "\x1b[1;38;5;" << static_cast<i32>(color) << "m";
	for (const char &c : body) {
		if (isprint(c) || c == '\n')
			std::cerr << c;
		else if (c == '\r')
			std::cerr << "\\r";
		else
			std::cerr << std::hex << "\\" << (static_cast<i32>(c) & 0xFF) << std::dec;
	}
	debug("\n}", color);
}

void ftMemset(void *dest, std::size_t count){
    unsigned char *p = (unsigned char*)dest;
    for (std::size_t i = 0; i < count; i++){
        p[i] = 0;
    }
}
