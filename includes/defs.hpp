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

#define HTTP_OK			200
#define HTTP_CREATED	201
#define HTTP_NO_CONTENT	204

#define HTTP_MOVED_PERMANENTLY	301
#define HTTP_FOUND				302

#define HTTP_BAD_REQUEST						400
#define HTTP_FORBIDDEN							403
#define HTTP_NOT_FOUND							404
#define HTTP_METHOD_NOT_ALLOWED					405
#define HTTP_REQUEST_TIMEOUT					408
#define HTTP_CONFLICT							409
#define HTTP_GONE								410
#define HTTP_LENGTH_REQUIRED					411
#define HTTP_PAYLOAD_TOO_LARGE					413
#define HTTP_URI_TOO_LONG						414
#define HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE	431

#define HTTP_INTERNAL_SERVER_ERROR	500
#define HTTP_NOT_IMPLEMENTED		501
#define HTTP_SERVICE_UNAVAILABLE	503
#define HTTP_VERSION_NOT_SUPPORTED	505

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
