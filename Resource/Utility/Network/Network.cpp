
#include "Network.h"
#include <ws2tcpip.h>
#include <mswsock.h>
#include <list>

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib, "Kernel32.lib")

//typedef BOOL(*LPFN_ACCEPTEX)(
//	SOCKET sListenSocket,
//	SOCKET sAcceptSocket,
//	PVOID lpOUtputBuffer,
//	DWORD dwReceiveDataLength,
//	DWORD dwLocalAddressLength,
//	DWORD dwRemoteAddressLength,
//	LPDWORD lpdwBytesReceived,
//	LPOVERLAPPED lpOverlapped
//	);

enum OPERATION_TYPE {
	ACCEPT,RECV
};

typedef struct _PER_IO_CONTEXT {
	OVERLAPPED   m_Overlapped;          // ÿһ���ص�I/O���������Ҫ��һ��                
	SOCKET       m_sockAccept;          // ���I/O������ʹ�õ�Socket��ÿ�����ӵĶ���һ����  
	WSABUF       m_wsaBuf;              // �洢���ݵĻ��������������ص��������ݲ����ģ�����WSABUF���滹�ὲ  
	char         m_szBuffer[1024]; // ��ӦWSABUF��Ļ�����  
	OPERATION_TYPE  m_OpType;               // ��־����ص�I/O��������ʲô�ģ�����Accept/Recv��  

} PER_IO_CONTEXT, *PPER_IO_CONTEXT;

typedef struct _PER_SOCKET_CONTEXT {
	SOCKET                   m_Socket;              // ÿһ���ͻ������ӵ�Socket  
	SOCKADDR_IN              m_ClientAddr;          // ����ͻ��˵ĵ�ַ  
	std::list<_PER_IO_CONTEXT*>  m_arrayIoContext;   // ���飬���пͻ���IO�����Ĳ�����  
												  // Ҳ����˵����ÿһ���ͻ���Socket  
												  // �ǿ���������ͬʱͶ�ݶ��IO�����  
} PER_SOCKET_CONTEXT, *PPER_SOCKET_CONTEXT;

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

bool network::Server::_start(int port, int max_connect = SOMAXCONN) {
	//Completion Port
	HANDLE CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,0);
	if(CompletionPort == NULL){
		WSACleanup();
		return false;
	}

	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo);
	HANDLE* WorkerThreads = new HANDLE[SysInfo.dwNumberOfProcessors * 2];
	memset(WorkerThreads, 0, sizeof(HANDLE)*SysInfo.dwNumberOfProcessors * 2);

	for (int i = 0; i < (SysInfo.dwNumberOfProcessors * 2); ++i) {
		WorkerThreads[i] = CreateThread(NULL, 0, ServerWorkThread, CompletionPort, 0, NULL);
		if (WorkerThreads[i] == NULL){
			//TODO CloseHandle
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
	SOCKET Sockid = WSASocket(AF_INET, SOCK_STREAM, 0,NULL,0,WSA_FLAG_OVERLAPPED);
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
	
	//ACCEPTEX
	LPFN_ACCEPTEX pAcceptEx;
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	DWORD dwBytes = 0;
	WSAIoctl(sockid, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, sizeof(GuidAcceptEx), &pAcceptEx, sizeof(pAcceptEx), &dwBytes, NULL, NULL);
	

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
