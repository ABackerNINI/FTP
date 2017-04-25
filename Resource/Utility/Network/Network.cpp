
#include "Network.h"

network::Server::Server() {
#if(DEBUG&DEBUG_TRACE)
	InitializeCriticalSection(&CRITICAL_PRINT);
#endif
}

bool network::Server::Start(config_server *cs, callback_server callback) {
	return _Start(cs->port, cs->max_connect);
}

bool network::Server::Stop() {
	_Stop(m_Sockid);

	return true;
}

bool network::Server::_InitComplitionPort() {
	//Completion Port
	m_CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_CompletionPort == NULL) {
		return false;
	}

	unsigned int _WorkerThreadsNum = _GetProcessorNum()*WORKER_THREADS_PER_PROCESSOR;

	HANDLE* _WorkerThreads = new HANDLE[_WorkerThreadsNum];
	memset(_WorkerThreads, 0, sizeof(HANDLE)*_WorkerThreadsNum);
	DWORD ThreadId;
	for (unsigned int i = 0; i < _WorkerThreadsNum; ++i) {
		WORKER_PARAMS *_pWorkerParams = new WORKER_PARAMS();
		_pWorkerParams->m_Server = this;
		_pWorkerParams->m_ThreadNo = i;

		_WorkerThreads[i] = CreateThread(NULL, 0, ServerWorkThread, _pWorkerParams, 0, &ThreadId);
		if (_WorkerThreads[i] == NULL) {
			//TODO CloseHandle
			return false;
		}
	}

	return true;
}

bool network::Server::_InitSock(int _Port, unsigned int _Max_Connect) {
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

	//PER_SOCKET_CONTEXT *_pSocketContext = new PER_SOCKET_CONTEXT();
	if (CreateIoCompletionPort((HANDLE)m_Sockid, m_CompletionPort, (ULONG_PTR)NULL, 0) == NULL) {
		//TODO
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

	//pACCEPTEX pGETACCEPTEXSOCKADDRS
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
	DWORD dwBytes = 0;
	if (SOCKET_ERROR == WSAIoctl(
		m_Sockid,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidAcceptEx,
		sizeof(GuidAcceptEx),
		&m_pAcceptEx,
		sizeof(m_pAcceptEx),
		&dwBytes,
		NULL,
		NULL)) {
		return false;
	}

	if (SOCKET_ERROR == WSAIoctl(
		m_Sockid,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidGetAcceptExSockAddrs,
		sizeof(GuidGetAcceptExSockAddrs),
		&m_pGetAcceptExSockAddrs,
		sizeof(m_pGetAcceptExSockAddrs),
		&dwBytes,
		NULL,
		NULL)) {
		return false;
	}

	//POST ACCEPT
	for (int i = 0; i < MAX_POST_ACCEPT; ++i) {//here exists a hard-to-solve bug with this frame
		PER_SOCKET_CONTEXT *_pSocketContext = new PER_SOCKET_CONTEXT();
		_PostAccept(_pSocketContext);
	}

	return true;
}

bool network::Server::_Start(int port, int max_connect = SOMAXCONN) {
	_InitComplitionPort();

	_InitSock(port, max_connect);

	return true;
}

void network::Server::_Stop(SOCKET sockid) {
	closesocket(sockid);
	WSACleanup();
}

bool network::Server::_PostAccept(PER_SOCKET_CONTEXT *_pSocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("PostAccept DEBUG_TRACE %d\n", _pSocketContext->_DEBUG_TRACE);
#endif

	DWORD _Flags = 0;

	_pSocketContext->m_OpType = ACCEPTED;
	_pSocketContext->m_ClientSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);

	if (m_pAcceptEx(m_Sockid,
		_pSocketContext->m_ClientSocket,
		_pSocketContext->m_wsaBuf.buf,
		_pSocketContext->m_wsaBuf.len - ((sizeof(SOCKADDR_IN) + 16) * 2),
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		&_Flags,
		&(_pSocketContext->m_Overlapped)) == false) {
		//TODO
		if (WSAGetLastError() != WSA_IO_PENDING) {
			return false;
		}
	}

	return true;
}

bool network::Server::_PostRecv(PER_SOCKET_CONTEXT *_pSocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("PostRecv DEBUG_TRACE %d\n", _pSocketContext->_DEBUG_TRACE);
#endif

	DWORD dwFlags = 0;
	DWORD dwBytes = 0;

	_pSocketContext->m_OpType = RECVING;

	int nBytesRecv = WSARecv(_pSocketContext->m_ClientSocket, &(_pSocketContext->m_wsaBuf), 1, &dwBytes, &dwFlags, &(_pSocketContext->m_Overlapped), NULL);

	if ((SOCKET_ERROR == nBytesRecv) && (WSA_IO_PENDING != WSAGetLastError())) {
		return false;
	}

	return true;
}

bool network::Server::_DoAccept(PER_SOCKET_CONTEXT *_pSocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("DoAccept DEBUG_TRACE %d\n", _pSocketContext->_DEBUG_TRACE);
#endif

	_Commit(_pSocketContext);

	PER_SOCKET_CONTEXT *_pNewSocketContex = new PER_SOCKET_CONTEXT();
	_pNewSocketContex->m_OpType = RECVING;
	_pNewSocketContex->m_ClientSocket = _pSocketContext->m_ClientSocket;
	SOCKADDR_IN *_ClientAddr, *_LocalAddr;//mark:need to delete?
	int _ClientAddrLen = sizeof(SOCKADDR_IN), _LocalAddrLen = sizeof(SOCKADDR_IN);

	m_pGetAcceptExSockAddrs(
		_pNewSocketContex->m_wsaBuf.buf,
		_pNewSocketContex->m_wsaBuf.len - ((sizeof(SOCKADDR_IN) + 16) * 2),
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		(LPSOCKADDR*)&_LocalAddr, &_LocalAddrLen,
		(LPSOCKADDR*)&_ClientAddr, &_ClientAddrLen);//mark:?

	//TODO save clientaddr

	if (CreateIoCompletionPort((HANDLE)_pNewSocketContex->m_ClientSocket, m_CompletionPort, (ULONG_PTR)_pNewSocketContex, 0) == NULL) {
		//TODO
		return false;
	}

	//_DoSend(_pNewSocketContex);

	_PostRecv(_pNewSocketContex);

	return _PostAccept(_pSocketContext);
}

bool network::Server::_DoRecv(PER_SOCKET_CONTEXT *_pSocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("DoRecv DEBUG_TRACE %d\n", _pSocketContext->_DEBUG_TRACE);
#endif
	//SOCKADDR_IN* ClientAddr = &_pSocketContext->m_ClientAddr;

	_Commit(_pSocketContext);

	_pSocketContext->m_OpType = RECVING;

	return _PostRecv(_pSocketContext);
}

bool network::Server::_DoSend(PER_SOCKET_CONTEXT *_pSocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("DoSend DEBUG_TRACE %d\n", _pSocketContext->_DEBUG_TRACE);
#endif
	//TODO WSASend();

	send(_pSocketContext->m_ClientSocket, _pSocketContext->m_szBuffer, _pSocketContext->m_BytesTransferred, 0);

	return true;
}
bool network::Server::_DoClose(PER_SOCKET_CONTEXT* _pSocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("DoClose DEBUG_TRACE %d\n", _pSocketContext->_DEBUG_TRACE);
#endif

	closesocket(_pSocketContext->m_ClientSocket);

	return true;
}

void network::Server::_Commit(PER_SOCKET_CONTEXT* _pSocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("Commit DEBUG_TRACE %d BytesTransferred:%u\n", _pSocketContext->_DEBUG_TRACE, _pSocketContext->m_BytesTransferred);
#endif

	if (_pSocketContext->m_BytesTransferred > 0) {
		//std::cout << _pSocketContext->m_szBuffer << std::endl;
		_pSocketContext->m_szBuffer[_pSocketContext->m_BytesTransferred] = '\0';

		printf("%s\n", _pSocketContext->m_szBuffer);

		_DoSend(_pSocketContext);

		//Sleep(5000);

		//_DoClose(_pSocketContext);

		_pSocketContext->RESET_BUFFER();
	}
}

DWORD WINAPI network::Server::ServerWorkThread(LPVOID IpParam) {
	WORKER_PARAMS *_WorkerParams = (WORKER_PARAMS*)IpParam;

#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("ServerWorkThread ThreadNo:%d\n", _WorkerParams->m_ThreadNo);
#endif

	DWORD _BytesTransferred;
	OVERLAPPED *_Overlapped;
	PER_SOCKET_CONTEXT *_pSocketContext;

	while (true) {
		_BytesTransferred = 0;

		if (GetQueuedCompletionStatus(_WorkerParams->m_Server->m_CompletionPort, &_BytesTransferred, (PULONG_PTR)&_pSocketContext, (LPOVERLAPPED*)&_Overlapped, INFINITE) == false) {
			continue;
		}

		_pSocketContext = CONTAINING_RECORD(_Overlapped, PER_SOCKET_CONTEXT, m_Overlapped);

		if (_BytesTransferred == 0 && (_pSocketContext->m_OpType == RECVING || _pSocketContext->m_OpType == SENDING)) {
			delete _pSocketContext;
			//TODO
			continue;
		}

		_pSocketContext->m_BytesTransferred = _BytesTransferred;

		switch (_pSocketContext->m_OpType) {
		case ACCEPTED:
			_WorkerParams->m_Server->_DoAccept(_pSocketContext);
			break;
		case RECVING:
			_WorkerParams->m_Server->_DoRecv(_pSocketContext);
			break;
		case SENDING:
			_WorkerParams->m_Server->_DoSend(_pSocketContext);
			break;
		default:
			break;
		}
	}

	return 0;
}

unsigned int network::Server::_GetProcessorNum() {
	SYSTEM_INFO SysInfo;
	GetSystemInfo(&SysInfo);

	return SysInfo.dwNumberOfProcessors;
}