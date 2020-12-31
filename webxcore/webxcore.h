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
//local headers
#include "RequestHandler.h"
#include "InternalDataTransit.h"

int csprint(std::string msg);

typedef struct client_data
{
	webxlib::csocket* cl;
	std::map<std::string, std::string> request_headers;
} cl_data;

class HTTPServer
{
public:
	HTTPServer(std::string cert_file, std::string xkey_file);
	virtual ~HTTPServer();

	void Start();
	void Stop();
protected:
	bool securelayer = false;
	bool svon = false;

	std::string certificate_file;
	std::string key_file;

	csockdata http_data;
	csockdata https_data;

	webxlib::csocket* httpsv = nullptr;
	webxlib::csocket* httpssv = nullptr;
};

typedef struct WEBXCOREIF
{
	webxlib* sv_interface			= nullptr;
	webxlib::webhook* web_interface = nullptr;
	HTTPServer* webxserver			= nullptr;
} WEBXCOREIF;

extern "C"
{
	WEBXCORE_API void StartWebServer();
	WEBXCORE_API void StopWebServer();
}
#endif //WEBXCORE_H
