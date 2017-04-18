#pragma once

#ifndef NINI_FTP_NETWORK_H
#define NINI_FTP_NETWORK_H

#include <stdio.h>
#include <WinSock2.h>
#include <iostream>
#include <windows.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <list>

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "Kernel32.lib")

namespace network {
	struct config_server {
		int port;
		int max_connect;
	};

	typedef void(*callback_server)(int ev, void *data);

	class Server {
	public:
		Server() {
		}
		bool start(config_server *cs, callback_server callback);
		bool stop();
	private:
		bool _start(int port, int max_connect);
		void _stop(SOCKET sockid);


	private:
		SOCKET sockid;
	};

	class Client {
	public:
		bool send();
	private:

	};
}
#endif //NINI_FTP_NETWORK_H