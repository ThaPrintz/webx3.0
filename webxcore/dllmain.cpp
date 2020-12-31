#include "RequestHandler.h"
#include <iostream>

DWORD WINAPI _requestproc(LPVOID pparam);
WEBXCOREIF* webx;
FILE* pFile;

DWORD WINAPI ConstructInternalDataTransit(LPVOID arg);
DataTransit* ConstructDataTransit();
DataTransit* _idt = nullptr;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call) 
	{
		case DLL_PROCESS_ATTACH:
			//pFile = nullptr;
			//AllocConsole();
			//freopen_s(&pFile, "CONOUT$", "w", stdout);

			webx = new WEBXCOREIF;
			webx->sv_interface = new webxlib();
			webx->web_interface = new webxlib::webhook();

			_idt = ConstructDataTransit();

			//CreateThread(NULL, NULL, ConstructInternalDataTransit, (LPVOID)_idt, 0, NULL);

			csprint("webxcore loaded");
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}

	return TRUE;
}

int csprint(std::string msg)
{
	return _idt->WriteData(msg.c_str(), msg.size());
}

DataTransit* ConstructDataTransit()
{
	csockdata _transitCfg;
	_transitCfg.address = "0.0.0.0";
	_transitCfg.port = "70";
	_transitCfg.dataprotocol = TCPSOCK;
	_transitCfg.ipprotocol = IPV4SOCK;

	DataTransit* _xidt = new DataTransit(&_transitCfg);

	return _xidt;
}

DWORD WINAPI ConstructInternalDataTransit(LPVOID arg)
{
	DataTransit* _midt = (DataTransit*)arg;

	_midt->EstablishTransit();

	return NULL;
}

HTTPServer::HTTPServer(std::string cert_file, std::string xkey_file)
{
	this->certificate_file = cert_file;
	this->key_file = xkey_file;

	this->https_data.address = "0.0.0.0";
	this->https_data.port = "443";
	this->https_data.dataprotocol = TCPSOCK;
	this->https_data.ipprotocol = IPV4SOCK;

	this->http_data.address = "0.0.0.0";
	this->http_data.port = "80";
	this->http_data.dataprotocol = TCPSOCK;
	this->http_data.ipprotocol = IPV4SOCK;
}

HTTPServer::~HTTPServer()
{
	delete this->httpsv;
	delete this->httpssv;
}

void HTTPServer::Start()
{
	this->httpsv = new webxlib::csocket(&this->http_data);
	this->httpssv = new webxlib::csocket(&this->https_data);

	this->svon = true;

	if (httpssv->Bind() != CSOCKET_SUCCESS)
		csprint("[webxcore] Server boot failed, master secure listening socket failed to Bind\n");

	if (httpssv->Listen() != CSOCKET_SUCCESS)
		csprint("[webxcore] Server boot failed, master secure listening socket failed to begin Listening\n");

	if (httpsv->Bind() != CSOCKET_SUCCESS)
		csprint("[webxcore] Server boot failed, master listening socket failed to Bind\n");

	if (httpsv->Listen() != CSOCKET_SUCCESS)
		csprint("[webxcore] Server boot failed, master listening socket failed to begin Listening\n");

	while (this->svon) {
		if (!this->httpsv->IsValid()) {
			csprint("[webxcore] critical error! Server HTTP Listener failed!\n");
			csprint("[webxcore] webx attempting to reboot server HTTP listener!\n");

			delete this->httpsv;
			this->httpsv = new webxlib::csocket(&this->http_data);
			if (this->httpsv->Bind() == CSOCKET_SUCCESS) {
				if (this->httpsv->Listen() == CSOCKET_SUCCESS) {
					csprint("[webxcore] webx rebooted server HTTP listener!\n");
				}
			} else {
				csprint("[webxcore] critical error! Server HTTP reboot failed!\n");
			}

			continue;
		} else if (!this->httpssv->IsValid()) {
			csprint("[webxcore] critical error! Server HTTPS Listener failed!\n");
			csprint("[webxcore] webx attempting to reboot server HTTPS listener!\n");

			delete this->httpssv;
			this->httpssv = new webxlib::csocket(&this->https_data);
			if (this->httpssv->Bind() == CSOCKET_SUCCESS) {
				if (this->httpssv->Listen() == CSOCKET_SUCCESS) {
					csprint("[webxcore] webx rebooted server HTTPS listener!\n");
				}
			} else {
				csprint("[webxcore] critical error! Server HTTPS reboot failed!\n");
			}

			continue;
		}

		if (this->httpsv->SelectReadable({ 0,0 }) > 0) {
			webxlib::csocket* client = this->httpsv->Accept();

			if (client->IsValid()) {
				CreateThread(NULL, NULL, _requestproc, (LPVOID)client, 0, NULL);
			} else {
				delete client;

				continue;
			}
		} 
		
		if (this->httpssv->SelectReadable({ 0,0 }) > 0) {
			webxlib::csocket* client = this->httpsv->Accept();
			if (client->IsValid()) {
				client->SSL_Init(this->certificate_file.c_str(), this->key_file.c_str());
				client->SSLBind();

				if (client->SSLAccept() == CSOCKET_SUCCESS) {
					CreateThread(NULL, NULL, _requestproc, (LPVOID)client, 0, NULL);
				}
			} else {
				delete client;

				continue;
			}
		}
	}
}

void HTTPServer::Stop()
{
	this->svon = false;
}

DWORD WINAPI _requestproc(LPVOID pparam)
{
	webxlib::csocket* client = (webxlib::csocket*)pparam;

	if (!client->IsValid()) 
		return NULL;
	
	char buff[1501];
	ZeroMemory(buff, 1501);

	while (int got = client->Recv(buff, 1500)) {
		if (got == CSOCKET_ERROR || strcmp(buff, "") == 0) {
			break;
		}
		
		cl_data* cl = new cl_data();
		cl->cl = client;
		cl->request_headers = webx->sv_interface->ParseHTTPRequest(buff);

		if (cl->request_headers["DATA"].substr(1, cl->request_headers["DATA"].size() - 1) == "favicon.ico")
			break;
		
		std::string cmsg = "method " + cl->request_headers["METHOD"] + " called on " + cl->request_headers["DATA"];

		csprint(cmsg);

		auto cll = cl->request_headers["DATA"].substr(1, cl->request_headers["DATA"].size() - 1);

		if (webx->web_interface->hookIsValid(cll)) {
			webx->web_interface->CallWebhook(cll, (void*)webx->sv_interface, (void*)cl);
		} else {
			webx->web_interface->CallWebhook("INDEX", (void*)webx->sv_interface, (void*)cl);
		}

		delete cl;
	}

	delete client;

	return NULL;
}

extern "C"
{
	WEBXCORE_API void StartWebServer()
	{
		webx->webxserver = new HTTPServer("certificate.crt", "private.key");
		webx->web_interface->RegisterWebhook("INDEX", _DEFAULTH);

		webx->webxserver->Start();
	}	

	WEBXCORE_API void StopWebServer()
	{
		webx->webxserver->Stop();

		delete webx->webxserver;
	}
}