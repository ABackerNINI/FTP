
#include <WinSock2.h>
#include "Network.h"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "Kernel32.lib")


// struct _WSADATA{
// public:
// 	_WSADATA(){
// 	}

// 	int WSAStartup(wVersionRequested){
// 		return WSAStartup(wVersionRequested, &_Wsadata)
// 	}

// 	int WSACleanup(){
// 		return WSACleanup();
// 	}

// 	~_WSADATA(){
// 		_WSADATA::WSACleanup();
// 	}
// public:
// 	WSADATA _Wsadata;
// };

bool network::Server::_start(int port, int max_connect) {
	//WSADATA
	WSADATA Wsadata;
	WORD wVersionRequested = MAKEWORD(2,2);
	if (WSAStartup(wVersionRequested, &Wsadata) != 0) {
		return false;
	}
	if(LOBYTE(Wsadata.wVersion)!=2 || HIBYTE(Wsadata.wVersion)!=2){
		WSACleanup();
		return false;
	}

	HANDLE CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,0);
	if(CompletionPort == NULL){
		WSACleanup();
		return false;
	}

	//SOCKET
	SOCKET sockid = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockid == INVALID_SOCKET) {
		WSACleanup();
		return false;
	}

	SOCKADDR_IN addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	if (bind(sockid, (SOCKADDR*)&addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		closesocket(sockid);
		WSACleanup();
		return false;
	}
	if (listen(sockid, max_connect)==SOCKET_ERROR) {
		closesocket(sockid);
		WSACleanup();
		return false;
	}



	//network::Server::sockid = sockid;

	return true;
}

void network::Server::_stop(SOCKET sockid) {
	closesocket(sockid);
	WSACleanup();
}