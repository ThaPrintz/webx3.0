#include "webserver.h"

DWORD WINAPI request_proc(LPVOID);

HTTPServer::HTTPServer(std::string cert_file, std::string xkey_file)
{
	this->certificate_file = cert_file;
	this->key_file = xkey_file;

	this->https_data.address		= "0.0.0.0";
	this->https_data.port			= "443";
	this->https_data.dataprotocol	= TCPSOCK;
	this->https_data.ipprotocol		= IPV4SOCK;
	this->https_data.socktype		= SECURESOCK;

	this->http_data.address			= "0.0.0.0";
	this->http_data.port			= "80";
	this->http_data.dataprotocol	= TCPSOCK;
	this->http_data.ipprotocol		= IPV4SOCK;
	this->http_data.socktype		= STANDARDSOCK;
}

void HTTPServer::Start()
{
	this->httpsv = new webxlib::csocket(&this->http_data);
	this->httpssv = new webxlib::csocket(&this->https_data);

	if (httpssv->Bind() != CSOCKET_SUCCESS)
		printf("[webx] Server boot failed, master secure listening socket failed to Bind\n");

	if (httpssv->Listen() != CSOCKET_SUCCESS)
		printf("[webx] Server boot failed, master secure listening socket failed to begin Listening\n");

	if (httpsv->Bind() != CSOCKET_SUCCESS)
		printf("[webx] Server boot failed, master listening socket failed to Bind\n");

	if (httpsv->Listen() != CSOCKET_SUCCESS)
		printf("[webx] Server boot failed, master listening socket failed to begin Listening\n");

	while (true) {
		if (!this->httpsv->IsValid()) {
			printf("[webx 2.0] critical error! Server HTTP Listener failed!\n");
			printf("[webx 2.0] webx attempting to reboot server HTTP listener!\n");

			delete this->httpsv;
			this->httpsv = new webxlib::csocket(&this->http_data);
			if (this->httpsv->Bind() == CSOCKET_SUCCESS) {
				if (this->httpsv->Listen() == CSOCKET_SUCCESS) {
					printf("[webx 2.0] webx rebooted server HTTP listener!\n");
				}
			} else {
				printf("[webx 2.0] critical error! Server HTTP reboot failed!\n");
			}

			continue;
		} else if (!this->httpssv->IsValid()) {
			printf("[webx 2.0] critical error! Server HTTPS Listener failed!\n");
			printf("[webx 2.0] webx attempting to reboot server HTTPS listener!\n");

			delete this->httpssv;
			this->httpssv = new webxlib::csocket(&this->https_data);
			if (this->httpssv->Bind() == CSOCKET_SUCCESS) {
				if (this->httpssv->Listen() == CSOCKET_SUCCESS) {
					printf("[webx 2.0] webx rebooted server HTTPS listener!\n");
				}
			} else {
				printf("[webx 2.0] critical error! Server HTTPS reboot failed!\n");
			}

			continue;
		}

		if (this->httpsv->SelectReadable({ 0,0 }) > 0) {
			webxlib::csocket* client = this->httpsv->Accept();
			if (client->IsValid()) {
				CreateThread(NULL, NULL, request_proc, (LPVOID)client, 0, NULL);
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
					CreateThread(NULL, NULL, request_proc, (LPVOID)client, 0, NULL);
				}
			} else {
				delete client;

				continue;
			}
		}
	}
}