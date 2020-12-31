#include "webxcore.h"
#include "defaults.h"

#include <Shlwapi.h>
#include <fstream>
#include <direct.h>

void* _DEFAULTH(void* arg, void* argp)
{
	webxlib* sv = (webxlib*)arg;
	cl_data* cl = (cl_data*)argp;
	
	//default routine for handling GET requests from client
	if (cl->request_headers["METHOD"] == "GET") {
		std::string data = cl->request_headers["DATA"];

		HTTP_packet resp;
		resp.server = std::string("webX 3.0 Web API Server");
		resp.connection = std::string("close");

		size_t filesz;

		//if request is asking for '/' or the main page send either 'index.html' if it exists, or webx default page
		if (strcmp(data.c_str(), "/") == 0) {
			resp.responsecode = std::string("200 OK");
			resp.content_type = std::string("text/html");

			if (sv->fileIsValid(webxindex)) {
				resp.response_content = std::string((char*)sv->LoadFile((char*)webxindex, &filesz));
				resp.content_length = std::to_string(filesz);
			} else {
				resp.response_content = std::string(mainpage);
				resp.content_length = std::to_string(strlen(mainpage));
			}
			
			auto reply = sv->BuildResponsePacket(resp.responsecode, resp.server, resp.content_length, resp.content_type, resp.connection, "");
			cl->cl->Send(reply.c_str(), reply.size());
			cl->cl->Send(resp.response_content.c_str(), resp.response_content.size());

			return NULL;
		//if the request isnt asking for the main page, determine wether its search for a file( and if it exists) or directory
		} else {
			data = cl->request_headers["DATA"].substr(1, cl->request_headers["DATA"].size() - 1);

			//if requested file exists in accessible directory, continue
			if (sv->fileIsValid(data.c_str())) {
				auto mimetypes = sv->GetMimetypesTable();
				std::string fType = PathFindExtensionA((LPCSTR)data.c_str());

				//if the file HAS an extension, continue
				if (fType != "\0") {
					fType = fType.substr(1, fType.size() - 1);

					//this means the files mimetype is not in our table, so default
					auto it = mimetypes.find(fType.data());
					if (it != mimetypes.end()) {
						resp.content_type = mimetypes[fType];
					} else {
						resp.content_type = std::string("application/octet-stream");
					}

					//client requested a range of data from a file
					if (cl->request_headers.find("Range") != cl->request_headers.end()) {
						auto bran = sv->strExplode(cl->request_headers["Range"], '-');

						std::streamsize st = std::stoi(bran[0]);
						std::streamsize end = std::stoi(bran[1]);

						std::fstream file(data.c_str(), std::fstream::in | std::fstream::out);
						file.seekg(st);

						char* fbuff = new char[(end - st) + 1];

						file.read(fbuff, end - st);
						file.close();

						auto clp = std::string("HTTP/1.1 206 Partial Content\r\nServer: webX 3.0 Web API Server\r\nDate: " + std::string(sv->systime()) + "\r\n"
							+ "Content-Length: " + std::to_string(end - st) + "\r\n"
							+ "Content-Type: " + resp.content_type + "\r\n"
							+ "Content-Range: bytes " + cl->request_headers["Range"] + "/*\r\n"
							+ "Connection: keep-alive\r\n\r\n"
							+ std::string(fbuff)
						);

						cl->cl->Send(clp.c_str(), clp.size());

						return NULL;
					}

					resp.responsecode = std::string("200 OK");
					resp.response_content = "";

					auto filed = (const char*)sv->LoadFile((char*)data.data(), &filesz);
					resp.content_length = std::to_string(filesz);

					auto reply = sv->BuildResponsePacket(resp.responsecode, resp.server, resp.content_length, resp.content_type, resp.connection, "");

					cl->cl->Send(reply.c_str(), reply.size());
					cl->cl->Send(filed, filesz);

					return NULL;
				//requested file does NOT have a file extension - so we will treat it like a directory
				} else {
					//get the filepath to the location of the web server client
					char cCurrentPath[FILENAME_MAX];
					char newdir[MAX_PATH];

					newdir[0] = '\0';

					if (_getcwd(cCurrentPath, sizeof(cCurrentPath))) {
						strncat_s(newdir, MAX_PATH, cCurrentPath, strlen(cCurrentPath));
						strncat_s(newdir, MAX_PATH, cl->request_headers["DATA"].data(), cl->request_headers["DATA"].length());
					}

					//format the newly concatted filepath + local requested directory to have backslashes instead of forward slashes
					for (int i = 0; i <= strlen(newdir); i++) {
						if (newdir[i] == '/') {

							newdir[i] = '\\';
						}
					}

					//add \\* on the end of the file path to indicate we want to iterate the entire directory
					strncat_s(newdir, MAX_PATH, "\\*", 2);

					//count how large the buffer needs to be to generate HTML
					WIN32_FIND_DATA FindFileData;
					int				fCount = 0;
					int				fNames = 0;

					//generate HTML for directory listing
					wchar_t filename[4096] = { 0 };
					MultiByteToWideChar(0, 0, newdir, strlen(newdir), filename, strlen(newdir));

					HANDLE hFindFile = FindFirstFile(filename, &FindFileData);
					if (hFindFile != INVALID_HANDLE_VALUE) {
						do {
							if (FindFileData.cFileName[0] == '.' && FindFileData.cFileName[1] == '\0' || FindFileData.cFileName[0] == '.' && FindFileData.cFileName[1] == '.') continue;

							std::wstring ws(FindFileData.cFileName);
							std::string dp(ws.begin(), ws.end());

							fCount++;
							fNames += ((int)cl->request_headers["DATA"].length() * 2) + ((int)dp.length() * 2) + 4;
						} while (FindNextFile(hFindFile, &FindFileData) != 0);

						FindClose(hFindFile);
					}

					//import canvas page HTML
					size_t fsize2;

					char* ret = (char*)sv->LoadFile((char*)"canvas1.html", &filesz);
					char* ret2 = (char*)sv->LoadFile((char*)"canvas2.html", &fsize2);

					//calculate size of buffer needed for HTML & create buffer
					int buffsz = filesz + fsize2 + 140 + (fNames)+(fCount * 95/*115 is the amount of characters in HTML each directory listing needs*/);

					char* trueret = new char[buffsz];
					trueret[0] = '\0';

					//copy first half of HTML canvas page to buffer
					strncat_s(trueret, buffsz, ret, filesz);

					//copy directory name HTML to buffer
					strncat_s(trueret, buffsz, "<h1 class=\"text-lowercase text-white font-weight-bold\">", strlen("<h1 class=\"text-lowercase text-white font-weight-bold\">"));
					strncat_s(trueret, buffsz, data.data(), data.length());
					strncat_s(trueret, buffsz, "/</h1><hr class=\"divider my-4\"></div><div class=\"col-lg-8 align-self-baseline\">", strlen("/</h1><hr class=\"divider my-4\"></div><div class=\"col-lg-8 align-self-baseline\">"));

					hFindFile = FindFirstFile(filename, &FindFileData);
					if (hFindFile != INVALID_HANDLE_VALUE) {
						do {
							std::wstring ws(FindFileData.cFileName);
							std::string dp(ws.begin(), ws.end());

							if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
								strncat_s(trueret, buffsz, "<div class=\"alert alert-info\" role=\"alert\"><a href=\"/", strlen("<div class=\"alert alert-info\" role=\"alert\">><a href=/\""));
								strncat_s(trueret, buffsz, data.data(), data.length());
								strncat_s(trueret, buffsz, "/", strlen("/"));
								strncat_s(trueret, buffsz, dp.c_str(), dp.length());
								strncat_s(trueret, buffsz, "\" class=\"alert-link\">", strlen("\" class=\"alert-link\">"));
								strncat_s(trueret, buffsz, data.data(), data.length());
								strncat_s(trueret, buffsz, "/", strlen("/"));
								strncat_s(trueret, buffsz, dp.c_str(), dp.length());
								strncat_s(trueret, buffsz, "</a></div>", strlen("</a></div>"));
							} else if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
								if (FindFileData.cFileName[0] == '.' && FindFileData.cFileName[1] == '\0' || FindFileData.cFileName[0] == '.' && FindFileData.cFileName[1] == '.') continue;

								std::vector<std::string>fp = sv->strExplode(dp, '\\');

								strncat_s(trueret, buffsz, "<div class=\"alert alert-info\" role=\"alert\"><a href=\"/", strlen("<div class=\"alert alert-info\" role=\"alert\">><a href=/\""));
								strncat_s(trueret, buffsz, data.data(), data.length());
								strncat_s(trueret, buffsz, "/", strlen("/"));
								strncat_s(trueret, buffsz, fp[fp.size() - 1].data(), fp[fp.size() - 1].length());
								strncat_s(trueret, buffsz, "\" class=\"alert-link\">", strlen("\" class=\"alert-link\">"));
								strncat_s(trueret, buffsz, data.data(), data.length());
								strncat_s(trueret, buffsz, "/", strlen("/"));
								strncat_s(trueret, buffsz, fp[fp.size() - 1].data(), fp[fp.size() - 1].length());
								strncat_s(trueret, buffsz, "/</a></div>", strlen("/</a></div>"));
							}
						} while (FindNextFile(hFindFile, &FindFileData) != 0);

						FindClose(hFindFile);
					}

					//copy second half of canvas page HTML to buffer
					//printf("\n_trueret: '%i'\n_ret2: '%i'\n_trueret + ret2: '%i'\n_buffsz: '%i'\n", strlen(trueret), strlen(ret2), strlen(trueret) + strlen(ret2), buffsz);
					strncat_s(trueret, buffsz, ret2, fsize2);

					resp.responsecode = std::string("200 OK");
					resp.content_type = std::string("text/html");
					resp.response_content = "";
					resp.content_length = std::to_string(strlen(trueret));

					auto reply = sv->BuildResponsePacket(resp.responsecode, resp.server, resp.content_length, resp.content_type, resp.connection, "");
					cl->cl->Send(reply.c_str(), reply.size());
					cl->cl->Send(trueret, (int)strlen(trueret));

					return NULL;
				}
			//file or directory does not exist, so 404
			} else {
				resp.responsecode = std::string("404 Not Found");
				resp.content_type = std::string("text/html");
				resp.response_content = "";

				if (sv->fileIsValid(fofcp)) {
					const char* sd = (char*)sv->LoadFile((char*)fofcp, &filesz);
					resp.content_length = std::to_string(filesz);

					auto reply = sv->BuildResponsePacket(resp.responsecode, resp.server, resp.content_length, resp.content_type, resp.connection, "");

					cl->cl->Send(reply.c_str(), reply.size());
					cl->cl->Send(sd, filesz);

					return NULL;
				}

				resp.content_length = std::to_string(strlen(FOURZEROFOUR));

				auto reply = sv->BuildResponsePacket(resp.responsecode, resp.server, resp.content_length, resp.content_type, resp.connection, "");
				cl->cl->Send(reply.c_str(), reply.size());
				cl->cl->Send(FOURZEROFOUR, strlen(FOURZEROFOUR));

				return NULL;
			}
		}
	} else if (cl->request_headers["METHOD"] == "HEAD") {
		std::string data = cl->request_headers["DATA"];

		HTTP_packet resp;
		resp.server = std::string("webX 3.0 Web API Server");
		resp.connection = std::string("close");

		size_t filesz;

		if (strcmp(data.c_str(), "/") == 0) {
			resp.responsecode = std::string("200 OK");
			resp.content_type = std::string("text/html");
			resp.response_content = "";

			if (sv->fileIsValid(webxindex)) {
				resp.response_content = (char*)sv->LoadFile((char*)webxindex, &filesz);
				resp.content_length = std::to_string(filesz);
			} else {
				resp.content_length = std::to_string(strlen(mainpage));
			}

			auto reply = sv->BuildResponsePacket(resp.responsecode, resp.server, resp.content_length, resp.content_type, resp.connection, "");
			cl->cl->Send(reply.c_str(), reply.size());

			return NULL;
		} else {
			data = cl->request_headers["DATA"].substr(1, cl->request_headers["DATA"].size() - 1);

			if (sv->fileIsValid(data.c_str())) {
				auto mimetypes = sv->GetMimetypesTable();
				std::string fType = PathFindExtensionA((LPCSTR)data.c_str());

				if (fType != "\0") {
					fType = fType.substr(1, fType.size() - 1);

					auto it = mimetypes.find(fType.data());
					if (it != mimetypes.end()) {
						resp.content_type = mimetypes[fType];
					} else {
						resp.content_type = std::string("application/octet-stream");
					}

					resp.responsecode = std::string("200 OK");
					resp.response_content = "";

					auto filed = (const char*)sv->LoadFile((char*)data.data(), &filesz);
					resp.content_length = std::to_string(filesz);

					auto reply = sv->BuildResponsePacket(resp.responsecode, resp.server, resp.content_length, resp.content_type, resp.connection, "");
					cl->cl->Send(reply.c_str(), reply.size());

					return NULL;
				} else {
					//404
					resp.responsecode = std::string("404 Not Found");
					resp.content_type = std::string("text/html");
					resp.response_content = "";
					resp.content_length = "0";

					auto reply = sv->BuildResponsePacket(resp.responsecode, resp.server, resp.content_length, resp.content_type, resp.connection, "");
					cl->cl->Send(reply.c_str(), reply.size());

					return NULL;
				}
			} else {
				//404
				resp.responsecode = std::string("404 Not Found");
				resp.content_type = std::string("text/html");
				resp.response_content = "";
				resp.content_length = "0";

				auto reply = sv->BuildResponsePacket(resp.responsecode, resp.server, resp.content_length, resp.content_type, resp.connection, "");
				cl->cl->Send(reply.c_str(), reply.size());

				return NULL;
			}
		}
	}/* else if (client->rheaders["METHOD"] == "POST") {
	} else if (client->rheaders["METHOD"] == "PUT") {
	} else if (client->rheaders["METHOD"] == "DELETE") {
	} else if (client->rheaders["METHOD"] == "CONNECT") {
	} else if (client->rheaders["METHOD"] == "OPTIONS") {
	} else if (client->rheaders["METHOD"] == "TRACE") {
	} else if (client->rheaders["METHOD"] == "PATCH") {
	}*/

	return nullptr;
}