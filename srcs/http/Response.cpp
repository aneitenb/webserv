// ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
// ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
// ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
// ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
// ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
//  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
//
// <<Response.cpp>> -- <<Aida, Ilmari, Milica>>

#include "http/Response.hpp"

static const std::map<int, std::string> statusMessages = {
	{200, "OK"},
	{201, "Created"},
	{204, "No Content"},
	{301, "Moved Permanently"},
	{302, "Found"},
	{400, "Bad Request"},
	{403, "Forbidden"},
	{404, "Not Found"},
	{405, "Method Not Allowed"},
	{413, "Payload Too Large"},
	{500, "Internal Server Error"},
	{501, "Not Implemented"},
	{503, "Service Unavailable"}
};

Response::Response(Request& request, ServerBlock* serverBlock)
	: _statusCode(200), _request(request), _serverBlock(serverBlock), _locationBlock(NULL)
{
	initializeMimeTypes();
}

Response::~Response() {}

void Response::initializeMimeTypes() {
	_mimeTypes = {
		{".html", "text/html"},
		{".htm", "text/html"},
		{".txt", "text/plain"},
		{".css", "text/css"},
		{".js", "application/javascript"},
		{".json", "application/json"},
		{".xml", "application/xml"},
		{".png", "image/png"},
		{".jpg", "image/jpeg"},
		{".jpeg", "image/jpeg"},
		{".gif", "image/gif"},
		{".svg", "image/svg+xml"},
		{".ico", "image/x-icon"},
		{".pdf", "application/pdf"},
		{".zip", "application/zip"},
		{".mp3", "audio/mpeg"},
		{".mp4", "video/mp4"}
	};
}
