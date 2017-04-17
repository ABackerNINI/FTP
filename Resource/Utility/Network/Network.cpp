
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

DWORD WINAPI ServerWorkThread(LPVOID CompletionPortID);
//DWORD WINAPI ServerSendThread(LPVOID IpParam);
HANDLE hMutex = CreateMutex(NULL, FALSE, NULL);

struct _SOCK {
	SOCKET Sockid;
	char Buffer[1024];
	unsigned int BufferLen;
};

bool network::Server::start(config_server *cs, callback_server callback) {
	return _start(cs->port, cs->max_connect);
}

bool network::Server::stop() {
	_stop(sockid);

	return true;
}

bool network::Server::_start(int port, int max_connect) {
	//Completion Port
	HANDLE CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,0);
	if(CompletionPort == NULL){
		WSACleanup();
		return false;
	}

	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo);
	HANDLE* WorkerThreads = new HANDLE[SysInfo.dwNumberOfProcessors * 2];

	for (int i = 0; i < (SysInfo.dwNumberOfProcessors * 2); ++i) {
		WorkerThreads[i] = CreateThread(NULL, 0, ServerWorkThread, CompletionPort, 0, NULL);
		if (WorkerThreads[i] == NULL) {
			return false;
		}
	}

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

	//SOCKET
	SOCKET Sockid = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (Sockid == INVALID_SOCKET) {
		WSACleanup();
		return false;
	}

	SOCKADDR_IN Addr;
	memset(&Addr, 0, sizeof(Addr));
	Addr.sin_family = AF_INET;
	Addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	Addr.sin_port = htons(port);

	if (bind(Sockid, (SOCKADDR*)&Addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		closesocket(Sockid);
		WSACleanup();
		return false;
	}
	if (listen(Sockid, max_connect)==SOCKET_ERROR) {
		closesocket(Sockid);
		WSACleanup();
		return false;
	}

	SOCKET RemoteSockid;
	SOCKADDR_IN RemoteAddr;
	int RemoteAddrLen;
	OVERLAPPED Overlapped;
	while (true) {
		RemoteAddrLen = sizeof(RemoteAddr);
		RemoteSockid = accept(Sockid, (SOCKADDR *)&RemoteAddr, &RemoteAddrLen);
		if (RemoteSockid == SOCKET_ERROR) {
			//mark:exit or continue?
			continue;
		}

		_SOCK *_Sock = (_SOCK*)GlobalAlloc(GPTR, sizeof(_SOCK));
		_Sock->Sockid = RemoteSockid;


		CreateIoCompletionPort((HANDLE)RemoteSockid, CompletionPort, (ULONG_PTR)_Sock, 0);


		DWORD RecvBytes,Flags = 0;


		WSARecv(RemoteSockid, (WSABUF *)&_Sock->Buffer, 1, &RecvBytes, &Flags, &Overlapped, NULL);
	}

	network::Server::sockid = sockid;

	return true;
}

void network::Server::_stop(SOCKET sockid) {
	closesocket(sockid);
	WSACleanup();
}

DWORD WINAPI ServerWorkThread(LPVOID IpParam) {

	HANDLE ComplitionPort = (HANDLE)IpParam;
	DWORD BytesTransferred;
	OVERLAPPED Overlapped;
	_SOCK *_Sock;

	while (true) {
		if (GetQueuedCompletionStatus(ComplitionPort, &BytesTransferred, (PULONG_PTR)&_Sock, (LPOVERLAPPED*)&Overlapped, INFINITE) == false) {
			return -1;
		}

		if (BytesTransferred == 0) {
			closesocket(_Sock->Sockid);
			GlobalFree(_Sock);
			continue;
		}
		WaitForSingleObject(hMutex, INFINITE);
		std::cout << _Sock->Buffer << std::endl;
		ReleaseMutex(hMutex);


	}

	return 0;
}
