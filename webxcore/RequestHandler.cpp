#include "RequestHandler.h"

void* _DEFAULTH(void* arg, void* argp)
{
	webxlib* sv = (webxlib*)arg;
	cl_data* cl = (cl_data*)argp;

	if (cl->request_headers["METHOD"] == "GET") {

	} /*else if (client->rheaders["METHOD"] == "HEAD") {
	} else if (client->rheaders["METHOD"] == "POST") {
	} else if (client->rheaders["METHOD"] == "PUT") {
	} else if (client->rheaders["METHOD"] == "DELETE") {
	} else if (client->rheaders["METHOD"] == "CONNECT") {
	} else if (client->rheaders["METHOD"] == "OPTIONS") {
	} else if (client->rheaders["METHOD"] == "TRACE") {
	} else if (client->rheaders["METHOD"] == "PATCH") {
	}*/
}