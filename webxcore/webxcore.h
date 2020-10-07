#pragma once
#define WIN32_LEAN_AND_MEAN

#ifdef WEBXCORE_EXPORTS
#define WEBXCORE_API __declspec(dllexport)
#else
#define WEBXCORE_API __declspec(dllimport)
#endif

#ifndef WEBXCORE_H
#define WEBXCORE_H

//HTTP framework that powers webx
#include "webxlib.h"

typedef struct client_data
{
	webxlib::csocket* cl;
	std::map<std::string, std::string> request_headers;
} cl_data;

typedef struct WEBXCORE_API WEBXCORE
{
	webxlib* sv_interface			= nullptr;
	webxlib::webhook* web_interface = nullptr;
	HTTPServer* webxserver			= nullptr;
};

void WEBXCORE_API StartWebServer();

//local headers
#include "webserver.h"
#include "RequestHandler.h"

#endif //WEBXCORE_H
