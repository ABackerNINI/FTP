
#include <WinSock2.h>
#include "Network.h"

#pragma comment(lib,"ws2_32.lib")//?


bool network::Server::_start(int port, int max_connect) {
	WSADATA Ws;
	SOCKET sockid;
	SOCKADDR_IN addr;

	if (WSAStartup(MAKEWORD(2, 2), &Ws) != 0) {
		return false;
	}

	sockid = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockid == INVALID_SOCKET) {
		return false;
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	bind(sockid, (SOCKADDR*)&addr, sizeof(SOCKADDR));
	listen(sockid, max_connect);

	network::Server::sockid = sockid;

	return true;
}

void network::Server::_stop(SOCKET sockid) {
	closesocket(sockid);
	WSACleanup();
}