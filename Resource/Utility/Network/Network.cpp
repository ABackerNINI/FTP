
#include "Network.h"

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

#define BUFFER_LEN 1024

enum OPERATION_TYPE {
	ACCEPT,RECV,SEND
};

typedef struct _PER_IO_CONTEXT {
	OVERLAPPED   m_Overlapped;          // 每一个重叠I/O网络操作都要有一个                
	SOCKET       m_sockAccept;          // 这个I/O操作所使用的Socket，每个连接的都是一样的  
	WSABUF       m_wsaBuf;              // 存储数据的缓冲区，用来给重叠操作传递参数的，关于WSABUF后面还会讲  
	char         m_szBuffer[BUFFER_LEN]; // 对应WSABUF里的缓冲区  
	OPERATION_TYPE  m_OpType;               // 标志这个重叠I/O操作是做什么的，例如Accept/Recv等  

} PER_IO_CONTEXT, *PPER_IO_CONTEXT;

typedef struct _PER_SOCKET_CONTEXT {
	SOCKET                   m_Socket;              // 每一个客户端连接的Socket  
	SOCKADDR_IN              m_ClientAddr;          // 这个客户端的地址  
	std::list<_PER_IO_CONTEXT*>  m_arrayIoContext;   // 数组，所有客户端IO操作的参数，  
												  // 也就是说对于每一个客户端Socket  
												  // 是可以在上面同时投递多个IO请求的  
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
	HANDLE CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (CompletionPort == NULL) {
		WSACleanup();
		return false;
	}

	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo);
	HANDLE* WorkerThreads = new HANDLE[SysInfo.dwNumberOfProcessors * 2];
	memset(WorkerThreads, 0, sizeof(HANDLE)*SysInfo.dwNumberOfProcessors * 2);

	for (int i = 0; i < (SysInfo.dwNumberOfProcessors * 2); ++i) {
		WorkerThreads[i] = CreateThread(NULL, 0, ServerWorkThread, CompletionPort, 0, NULL);
		if (WorkerThreads[i] == NULL) {
			//TODO CloseHandle
			return false;
		}
	}

	//WSADATA
	WSADATA Wsadata;
	WORD wVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(wVersionRequested, &Wsadata) != 0) {
		return false;
	}
	if (LOBYTE(Wsadata.wVersion) != 2 || HIBYTE(Wsadata.wVersion) != 2) {
		WSACleanup();
		return false;
	}

	//SOCKET
	SOCKET Sockid = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (Sockid == INVALID_SOCKET) {
		WSACleanup();
		return false;
	}

	_PER_SOCKET_CONTEXT *PerSocketContext = (_PER_SOCKET_CONTEXT *)GlobalAlloc(GPTR, sizeof(_PER_SOCKET_CONTEXT));
	PerSocketContext->m_Socket = Sockid;

	CreateIoCompletionPort((HANDLE)Sockid, CompletionPort, (ULONG_PTR)PerSocketContext, 0);

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
	if (listen(Sockid, max_connect) == SOCKET_ERROR) {
		closesocket(Sockid);
		WSACleanup();
		return false;
	}

	//ACCEPTEX
	LPFN_ACCEPTEX pAcceptEx;
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	DWORD dwBytes = 0;
	WSAIoctl(Sockid, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, sizeof(GuidAcceptEx), &pAcceptEx, sizeof(pAcceptEx), &dwBytes, NULL, NULL);

	//
	DWORD flags = 0;
	//while (true) {
	_PER_IO_CONTEXT *PerIoContext = (_PER_IO_CONTEXT *)GlobalAlloc(GPTR, sizeof(_PER_IO_CONTEXT));
	memset(&(PerIoContext->m_Overlapped), 0, sizeof(OVERLAPPED));
	PerIoContext->m_OpType = ACCEPT;
	PerIoContext->m_sockAccept = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	int rc = pAcceptEx(Sockid, PerIoContext->m_sockAccept, PerIoContext->m_szBuffer, BUFFER_LEN - ((sizeof(SOCKADDR_IN) + 16) * 2), sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, &flags, &(PerIoContext->m_Overlapped));
	if (rc == 0) {
		//TODO
	}
	//}

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
	_PER_SOCKET_CONTEXT *PerSocketContext;
	_PER_IO_CONTEXT *PerIoContext;
	DWORD flags;

	while (true) {
		BytesTransferred = 0;

		if (GetQueuedCompletionStatus(ComplitionPort, &BytesTransferred, (PULONG_PTR)&PerSocketContext, (LPOVERLAPPED*)&PerIoContext, INFINITE) == false) {
			return -1;
		}

		if (BytesTransferred == 0 && (PerIoContext->m_OpType == RECV || PerIoContext->m_OpType == SEND)) {
			closesocket(PerSocketContext->m_Socket);
			GlobalFree(PerSocketContext);
			GlobalFree(PerIoContext);
			continue;
		}

		if (PerIoContext->m_OpType == ACCEPT) {
			if (setsockopt(PerIoContext->m_sockAccept, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char *)&(PerSocketContext->m_Socket), sizeof(PerSocketContext->m_Socket)) == SOCKET_ERROR) {
				//todo
			}

			PerSocketContext->m_Socket = PerIoContext->m_sockAccept;
			CreateIoCompletionPort((HANDLE)PerSocketContext->m_Socket, ComplitionPort, (ULONG_PTR)PerSocketContext, 0);
			memset(&PerIoContext->m_Overlapped, 0, sizeof(OVERLAPPED));
			PerIoContext->m_OpType = RECV;

			PerIoContext->m_wsaBuf.buf = PerIoContext->m_szBuffer;
			PerIoContext->m_wsaBuf.len = BUFFER_LEN;

			flags = 0;
			if (WSARecv(PerSocketContext->m_Socket, &(PerIoContext->m_wsaBuf), 1, &BytesTransferred, &flags, &(PerIoContext->m_Overlapped), NULL) == SOCKET_ERROR) {
				if (WSAGetLastError() == WSA_IO_PENDING) {
					//todo
				}
			}

			continue;
		}

		if (PerIoContext->m_OpType == RECV) {
			std::cout << PerIoContext->m_wsaBuf.buf << std::endl;
		}

		flags = 0;
		PerIoContext->m_OpType = RECV;
		memset(&(PerIoContext->m_Overlapped), 0, sizeof(OVERLAPPED));

		WSARecv(PerSocketContext->m_Socket, &(PerIoContext->m_wsaBuf), 1, &BytesTransferred, &flags, &(PerIoContext->m_Overlapped), NULL);
	}

	return 0;
}
