#pragma once
#include "webxcore.h"

#ifndef REQHANDLER_H
#define REQHANDLER_H

typedef struct HTTP_packet
{
	std::string responsecode;
	std::string server;
	std::string date;
	std::string content_length;
	std::string content_type;
	std::string connection;
	std::string response_content;
} HTTP_packet;

typedef struct WEBSOCK_packet
{
	std::string responsecode;
	std::string server;
	std::string date;
	std::string upgrade;
	std::string connection;
	std::string secwsaccept;
	std::string secwsproto;
};

static void* _DEFAULT(void* arg, void* argp);

#endif //REQHANDLER_H