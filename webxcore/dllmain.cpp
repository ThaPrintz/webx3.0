#include "RequestHandler.h"

DWORD WINAPI _requestproc(LPVOID pparam);
WEBXCOREIF* webx;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	webx = new WEBXCOREIF;

	webx->sv_interface	= new webxlib();
	webx->web_interface = new webxlib::webhook();
	webx->webxserver	= new HTTPServer("certificate.crt", "private.key");

	webx->web_interface->RegisterWebhook("INDEX", _DEFAULTH);

	return TRUE;
}

HTTPServer::HTTPServer(std::string cert_file, std::string xkey_file)
{
	this->certificate_file = cert_file;
	this->key_file = xkey_file;

	this->https_data.address = "0.0.0.0";
	this->https_data.port = "443";
	this->https_data.dataprotocol = TCPSOCK;
	this->https_data.ipprotocol = IPV4SOCK;
	this->https_data.socktype = SECURESOCK;

	this->http_data.address = "0.0.0.0";
	this->http_data.port = "80";
	this->http_data.dataprotocol = TCPSOCK;
	this->http_data.ipprotocol = IPV4SOCK;
	this->http_data.socktype = STANDARDSOCK;
}

void HTTPServer::Start()
{
	this->httpsv = new webxlib::csocket(&this->http_data);
	this->httpssv = new webxlib::csocket(&this->https_data);

	this->svon = true;

	if (httpssv->Bind() != CSOCKET_SUCCESS)
		printf("[webx] Server boot failed, master secure listening socket failed to Bind\n");

	if (httpssv->Listen() != CSOCKET_SUCCESS)
		printf("[webx] Server boot failed, master secure listening socket failed to begin Listening\n");

	if (httpsv->Bind() != CSOCKET_SUCCESS)
		printf("[webx] Server boot failed, master listening socket failed to Bind\n");

	if (httpsv->Listen() != CSOCKET_SUCCESS)
		printf("[webx] Server boot failed, master listening socket failed to begin Listening\n");

	while (this->svon) {
		if (!this->httpsv->IsValid()) {
			printf("[webx 3.0] critical error! Server HTTP Listener failed!\n");
			printf("[webx 3.0] webx attempting to reboot server HTTP listener!\n");

			delete this->httpsv;
			this->httpsv = new webxlib::csocket(&this->http_data);
			if (this->httpsv->Bind() == CSOCKET_SUCCESS) {
				if (this->httpsv->Listen() == CSOCKET_SUCCESS) {
					printf("[webx 3.0] webx rebooted server HTTP listener!\n");
				}
			} else {
				printf("[webx 3.0] critical error! Server HTTP reboot failed!\n");
			}

			continue;
		} else if (!this->httpssv->IsValid()) {
			printf("[webx 3.0] critical error! Server HTTPS Listener failed!\n");
			printf("[webx 3.0] webx attempting to reboot server HTTPS listener!\n");

			delete this->httpssv;
			this->httpssv = new webxlib::csocket(&this->https_data);
			if (this->httpssv->Bind() == CSOCKET_SUCCESS) {
				if (this->httpssv->Listen() == CSOCKET_SUCCESS) {
					printf("[webx 3.0] webx rebooted server HTTPS listener!\n");
				}
			} else {
				printf("[webx 3.0] critical error! Server HTTPS reboot failed!\n");
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
		} else if (this->httpssv->SelectReadable({ 0,0 }) > 0) {
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
	auto shut = false;

	while (!shut) {
		int got = client->Recv(buff, 1500);
		if (got == CSOCKET_ERROR || strcmp(buff, "") == 0) {
			break;
		}

		printf("\n%s\n", buff);

		cl_data cl;
		cl.cl = client;
		cl.request_headers = webx->sv_interface->ParseHTTPRequest(buff);

		if (cl.request_headers["DATA"].substr(1, cl.request_headers["DATA"].size() - 1) == "favicon.ico")
			break;

		auto cll = cl.request_headers["DATA"].substr(1, cl.request_headers["DATA"].size() - 1);

		if (webx->web_interface->hookIsValid(cll)) {
			webx->web_interface->CallWebhook(cll, (void*)webx->sv_interface, (void*)&cl);
		} else {
			webx->web_interface->CallWebhook("INDEX", (void*)webx->sv_interface, (void*)&cl);
		}

		shut = true;
	}

	delete client;

	return NULL;
}