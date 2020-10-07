#pragma once
#include "webxcore.h"

#ifndef WEBSEVER_H
#define WEBSERVER_H

class HTTPServer
{
public:
	HTTPServer(std::string cert_file, std::string xkey_file);
	virtual ~HTTPServer();

	void Start();
	void Stop();
protected:
	bool securelayer = false;
	static std::string certificate_file;
	static std::string key_file;

	csockdata http_data;
	csockdata https_data;

	webxlib::csocket* httpsv	= nullptr;
	webxlib::csocket* httpssv	= nullptr;
};

#endif //WEBSERVER_H