#include "webxcore.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	WEBXCORE* webx;

	webx->sv_interface	= new webxlib();
	webx->web_interface = new webxlib::webhook();
	webx->webxserver	= new HTTPServer("certificate.crt", "private.key");

	return TRUE;
}