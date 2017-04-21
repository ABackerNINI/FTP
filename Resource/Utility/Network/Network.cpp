
#include "Network.h"

#define _SERVER network::Server

DWORD WINAPI ServerWorkThread(LPVOID CompletionPortID);
//DWORD WINAPI ServerSendThread(LPVOID IpParam);
//HANDLE hMutex = CreateMutex(NULL, FALSE, NULL);

_SERVER::Server() {

}

bool _SERVER::Start(config_server *cs, callback_server callback) {
	return _Start(cs->port, cs->max_connect);
}

bool _SERVER::Stop() {
	_Stop(m_Sockid);

	return true;
}

bool _SERVER::_InitSock(int _Port, unsigned int _Max_Connect) {
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
	m_Sockid = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_Sockid == INVALID_SOCKET) {
		WSACleanup();
		return false;
	}

	SOCKADDR_IN _Addr;
	memset(&_Addr, 0, sizeof(_Addr));
	_Addr.sin_family = AF_INET;
	_Addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	_Addr.sin_port = htons(_Port);

	//BIND
	if (bind(m_Sockid, (SOCKADDR*)&_Addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		closesocket(m_Sockid);
		WSACleanup();
		return false;
	}

	//LISTEN
	if (listen(m_Sockid, _Max_Connect) == SOCKET_ERROR) {
		closesocket(m_Sockid);
		WSACleanup();
		return false;
	}

	return true;
}

bool _SERVER::_InitComplitionPort(HANDLE *_CompletionPort) {
	//Completion Port
	*_CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (*_CompletionPort == NULL) {
		return false;
	}

	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo);

	unsigned int _WorkerThreadsNum = SysInfo.dwNumberOfProcessors * 2;
	HANDLE* _WorkerThreads = new HANDLE[];
	memset(_WorkerThreads, 0, sizeof(HANDLE)*_WorkerThreadsNum);
	DWORD ThreadId;
	for (int i = 0; i < _WorkerThreadsNum; ++i) {
		_WorkerThreads[i] = CreateThread(NULL, 0, ServerWorkThread, *_CompletionPort, 0, &ThreadId);
		if (_WorkerThreads[i] == NULL) {
			//TODO CloseHandle
			return false;
		}
	}

	return false;
}

bool _InitAcceptEx() {}

bool _SERVER::_Start(int port, int max_connect = SOMAXCONN) {

	HANDLE _CompletionPort;
	_InitComplitionPort(&_CompletionPort);

	_InitSock(port,max_connect);

	_PER_SOCKET_CONTEXT *PerSocketContext = (_PER_SOCKET_CONTEXT *)GlobalAlloc(GPTR, sizeof(_PER_SOCKET_CONTEXT));
	PerSocketContext->m_Socket = m_Sockid;

	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;

	CreateIoCompletionPort((HANDLE)m_Sockid, _CompletionPort, (ULONG_PTR)PerSocketContext, 0);



	_SERVER::m_Sockid = m_Sockid;

	return true;
}

void _SERVER::_Stop(SOCKET sockid) {
	closesocket(sockid);
	WSACleanup();
}

bool _SERVER::_PostAccept(_PER_IO_CONTEXT *_PerIoContext) {
	DWORD _Flags = 0;

	_PER_IO_CONTEXT *PerIoContext = (_PER_IO_CONTEXT *)GlobalAlloc(GPTR, sizeof(_PER_IO_CONTEXT));
	memset(&(PerIoContext->m_Overlapped), 0, sizeof(OVERLAPPED));
	PerIoContext->m_OpType = ACCEPT;
	PerIoContext->m_wsaBuf.buf = PerIoContext->m_szBuffer;
	PerIoContext->m_wsaBuf.len = BUFFER_LEN;
	PerIoContext->m_sockAccept = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);

	int rc = m_pAcceptEx(_PerIoContext->m_sockAccept,
		PerIoContext->m_sockAccept,
		PerIoContext->m_szBuffer,
		PerIoContext->m_wsaBuf.len - ((sizeof(SOCKADDR_IN) + 16) * 2),
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		&_Flags,
		&(PerIoContext->m_Overlapped)
	);
	if (rc == 0) {
		//TODO
	}
}

bool _SERVER::_PostRecv() {}

bool _SERVER::_DoAccept() {}

bool _SERVER::_DoRecv() {

}

DWORD WINAPI ServerWorkThread(LPVOID IpParam) {
	HANDLE ComplitionPort = (HANDLE)IpParam;
	DWORD BytesTransferred;
	OVERLAPPED *Overlapped;
	_PER_SOCKET_CONTEXT *PerSocketContext;
	_PER_IO_CONTEXT *PerIoContext;
	DWORD flags;

	while (true) {
		BytesTransferred = 0;

		if (GetQueuedCompletionStatus(ComplitionPort, &BytesTransferred, (PULONG_PTR)&PerSocketContext, (LPOVERLAPPED*)&Overlapped, INFINITE) == false) {
			return -1;
		}

		PerIoContext = CONTAINING_RECORD(Overlapped, PER_IO_CONTEXT, m_Overlapped);

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
