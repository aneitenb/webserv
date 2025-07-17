// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<defs.hpp>> -- <<Aida, Ilmari, Milica>>

#pragma once

#include <cstdint>

#define CR	"\r"
#define LF	"\n"
#define SP	" "
#define HT	"\t"

#define CRLF CR LF
#define LWS CRLF SP HT

#define SGR_RESET	"\x1b[m"

#ifdef __DEBUG
 #ifndef __DEBUG_NO_REQ
  #ifndef __DEBUG_NO_REQ_HEADERS
   #define __DEBUG_REQ_SHOW_HEADERS
  #endif /* __DEBUG_NO_REQ_HEADERS */
  #ifndef __DEBUG_NO_REQ_BODY
   #define __DEBUG_REQ_SHOW_BODY
  #endif /* __DEBUG_NO_REQ_BODY */
 #endif /* __DEBUG_NO_REQ */
 #ifndef __DEBUG_NO_RES
  #ifndef __DEBUG_NO_RES_HEADERS
   #define __DEBUG_RES_SHOW_HEADERS
  #endif /* __DEBUG_NO_RES_HEADERS */
  #ifndef __DEBUG_NO_RES_BODY
   #define __DEBUG_RES_SHOW_BODY
  #endif /* __DEBUG_NO_RES_BODY */
 #endif /* __DEBUG_NO_RES */
#endif /* __DEBUG */

typedef int8_t		i8;
typedef int16_t		i16;
typedef int32_t		i32;
typedef int64_t		i64;

typedef uint8_t		u8;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef uint64_t	u64;
