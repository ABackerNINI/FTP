
#include "Network.h"

/*-------------------------------------Server Section-------------------------------------*/

network::Server::Server() {
#if(DEBUG&DEBUG_TRACE)
	InitializeCriticalSection(&CRITICAL_PRINT);
#endif
}

network::Server::Server(const ServerConfig &_ServerConfig) {
#if(DEBUG&DEBUG_TRACE)
	InitializeCriticalSection(&CRITICAL_PRINT);
#endif

	m_ServerConfig = _ServerConfig;
}

void network::Server::SetConfig(const ServerConfig &_ServerConfig) {
	m_ServerConfig = _ServerConfig;
}

bool network::Server::Start() {
	return _Start(m_ServerConfig.M_Port, m_ServerConfig.O_MaxConnect);
}

bool network::Server::Send(SOCKET _Socket, const char * _SendBuffer, unsigned int _BufferLen) {
	SVR_SOCKET_CONTEXT *_SocketContext = new SVR_SOCKET_CONTEXT(_Socket, _SendBuffer, _BufferLen);

	return _PostSend(_SocketContext);
}

bool network::Server::Close(SOCKET _Socket) {
	closesocket(_Socket);

	return true;
}

bool network::Server::Stop() {
	_Stop(m_Socket);

	return true;
}

void network::Server::OnAccepted(SVR_SOCKET_CONTEXT * _SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("OnAccepted DEBUG_TRACE %u BytesTransferred:%u\n", _SocketContext->_DEBUG_TRACE, _SocketContext->m_BytesTransferred);
#endif

	printf("%s\n", _SocketContext->m_szBuffer);
}

void network::Server::OnRecvd(SVR_SOCKET_CONTEXT * _SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("OnRecvd DEBUG_TRACE %u BytesTransferred:%u\n", _SocketContext->_DEBUG_TRACE, _SocketContext->m_BytesTransferred);
#endif

	printf("%s\n", _SocketContext->m_szBuffer);
}

void network::Server::OnSent(SVR_SOCKET_CONTEXT * _SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("OnSent DEBUG_TRACE %u BytesTransferred:%u\n", _SocketContext->_DEBUG_TRACE, _SocketContext->m_BytesTransferred);
#endif
}

void network::Server::OnClosed(SVR_SOCKET_CONTEXT * _SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("OnClosed DEBUG_TRACE %u BytesTransferred:%u\n", _SocketContext->_DEBUG_TRACE, _SocketContext->m_BytesTransferred);
#endif
}

bool network::Server::_InitComplitionPort() {
	//Completion Port
	m_CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_CompletionPort == NULL) {
		return false;
	}

	unsigned int _WorkerThreadsNum = _GetProcessorNum()*m_ServerConfig.O_WorkerThreadsPerProcessor;

	HANDLE* _WorkerThreads = new HANDLE[_WorkerThreadsNum];
	memset(_WorkerThreads, 0, sizeof(HANDLE)*_WorkerThreadsNum);
	DWORD ThreadId;
	for (unsigned int i = 0; i < _WorkerThreadsNum; ++i) {
		WORKER_PARAMS<Server> *_pWorkerParams = new WORKER_PARAMS<Server>();
		_pWorkerParams->m_Instance = this;
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
		return false;
	}

	//SOCKET
	m_Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_Socket == INVALID_SOCKET) {
		return false;
	}

	SOCKADDR_IN _Addr;
	memset(&_Addr, 0, sizeof(_Addr));
	_Addr.sin_family = AF_INET;
	_Addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	_Addr.sin_port = htons(_Port);

	//BIND
	if (bind(m_Socket, (SOCKADDR*)&_Addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		return false;
	}

	//LISTEN
	if (listen(m_Socket, _Max_Connect) == SOCKET_ERROR) {
		return false;
	}

	if (CreateIoCompletionPort((HANDLE)m_Socket, m_CompletionPort, (ULONG_PTR)NULL, 0) == NULL) {
		return false;
	}

	//pACCEPTEX pGETACCEPTEXSOCKADDRS
	GUID GuidAcceptEx = WSAID_ACCEPTEX;
	GUID GuidGetAcceptExSockAddrs = WSAID_GETACCEPTEXSOCKADDRS;
	DWORD dwBytes = 0;
	if (SOCKET_ERROR == WSAIoctl(
		m_Socket,
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
		m_Socket,
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
	for (int i = 0; i < m_ServerConfig.O_MaxPostAccept; ++i) {
		SVR_SOCKET_CONTEXT *_SocketContext = new SVR_SOCKET_CONTEXT();
		_PostAccept(_SocketContext);
	}

	return true;
}

bool network::Server::_Start(int _Port, int _MaxConnect = SOMAXCONN) {
	_InitComplitionPort();

	_InitSock(_Port, _MaxConnect);

	return true;
}

void network::Server::_Stop(SOCKET _Sockid) {
	closesocket(_Sockid);
	WSACleanup();
}

bool network::Server::_PostAccept(SVR_SOCKET_CONTEXT *_SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("PostAccept DEBUG_TRACE %u\n", _SocketContext->_DEBUG_TRACE);
#endif

	DWORD _Flags = 0;

	_SocketContext->m_OpType = SVR_OP::SVROP_ACCEPTING;
	_SocketContext->m_ClientSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);

	if (m_pAcceptEx(m_Socket,
		_SocketContext->m_ClientSocket,
		_SocketContext->m_wsaBuf.buf,
		_SocketContext->m_wsaBuf.len - ((sizeof(SOCKADDR_IN) + 16) * 2),
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		&_Flags,
		&(_SocketContext->m_Overlapped)) == false) {
		//TODO
		if (WSAGetLastError() != WSA_IO_PENDING) {
			return false;
		}
	}

	return true;
}

bool network::Server::_PostRecv(SVR_SOCKET_CONTEXT *_SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("PostRecv DEBUG_TRACE %u\n", _SocketContext->_DEBUG_TRACE);
#endif

	DWORD dwFlags = 0;
	DWORD dwBytes = 0;

	_SocketContext->m_OpType = SVR_OP::SVROP_RECVING;

	if (WSARecv(_SocketContext->m_ClientSocket, &(_SocketContext->m_wsaBuf), 1, &dwBytes, &dwFlags, &(_SocketContext->m_Overlapped), NULL) == SOCKET_ERROR &&
		WSAGetLastError() != WSA_IO_PENDING) {
		return false;
	}

	return true;
}

bool network::Server::_PostSend(SVR_SOCKET_CONTEXT *_SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("PostSend DEBUG_TRACE %u\n", _SocketContext->_DEBUG_TRACE);
#endif

	DWORD dwFlags = 0;
	DWORD dwBytes = 0;

	_SocketContext->m_OpType = SVR_OP::SVROP_SENDING;

	if (WSASend(_SocketContext->m_ClientSocket, &(_SocketContext->m_wsaBuf), 1, &dwBytes, dwFlags, &(_SocketContext->m_Overlapped), NULL) == SOCKET_ERROR &&
		WSAGetLastError() != WSA_IO_PENDING) {
		return false;
	}

	return true;
}

bool network::Server::_DoAccepted(SVR_SOCKET_CONTEXT *_SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("DoAccept DEBUG_TRACE %u Socket:%d\n", _SocketContext->_DEBUG_TRACE, _SocketContext->m_ClientSocket);
#endif

	_SocketContext->m_szBuffer[_SocketContext->m_BytesTransferred] = '\0';

	OnAccepted(_SocketContext);

	_SocketContext->RESET_BUFFER();

	SVR_SOCKET_CONTEXT *_NewSocketContex = new SVR_SOCKET_CONTEXT();
	_NewSocketContex->m_OpType = SVR_OP::SVROP_RECVING;
	_NewSocketContex->m_ClientSocket = _SocketContext->m_ClientSocket;
	SOCKADDR_IN *_ClientAddr, *_LocalAddr;//mark:need to delete?
	int _ClientAddrLen = sizeof(SOCKADDR_IN), _LocalAddrLen = sizeof(SOCKADDR_IN);

	m_pGetAcceptExSockAddrs(
		_NewSocketContex->m_wsaBuf.buf,
		_NewSocketContex->m_wsaBuf.len - ((sizeof(SOCKADDR_IN) + 16) * 2),
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		(LPSOCKADDR*)&_LocalAddr, &_LocalAddrLen,
		(LPSOCKADDR*)&_ClientAddr, &_ClientAddrLen);//mark:?

	//TODO save clientaddr

	if (CreateIoCompletionPort((HANDLE)_NewSocketContex->m_ClientSocket, m_CompletionPort, (ULONG_PTR)_NewSocketContex, 0) == NULL) {
		//TODO
		return false;
	}

	_PostRecv(_NewSocketContex);

	_PostAccept(_SocketContext);

	return true;
}

bool network::Server::_DoRecvd(SVR_SOCKET_CONTEXT *_SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("DoRecv DEBUG_TRACE %u\n", _SocketContext->_DEBUG_TRACE);
#endif

	_SocketContext->m_szBuffer[_SocketContext->m_BytesTransferred] = '\0';

	OnRecvd(_SocketContext);

	_SocketContext->RESET_BUFFER();

	_SocketContext->m_OpType = SVR_OP::SVROP_RECVING;

	return _PostRecv(_SocketContext);
}

bool network::Server::_DoSent(SVR_SOCKET_CONTEXT *_SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("DoSend DEBUG_TRACE %u\n", _SocketContext->_DEBUG_TRACE);
#endif

	OnSent(_SocketContext);

	delete _SocketContext;//TODO use repeatedly

	return true;
}

DWORD WINAPI network::Server::ServerWorkThread(LPVOID _LpParam) {
	WORKER_PARAMS<Server> *_WorkerParams = (WORKER_PARAMS<Server>*)_LpParam;

#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("ServerWorkThread ThreadNo:%u\n", _WorkerParams->m_ThreadNo);
#endif

	DWORD _BytesTransferred;
	OVERLAPPED *_Overlapped;
	SVR_SOCKET_CONTEXT *_SocketContext;

	while (true) {
		_BytesTransferred = 0;

		if (GetQueuedCompletionStatus(_WorkerParams->m_Instance->m_CompletionPort, &_BytesTransferred, (PULONG_PTR)&_SocketContext, (LPOVERLAPPED*)&_Overlapped, INFINITE) == false) {
			continue;
		}

		_SocketContext = CONTAINING_RECORD(_Overlapped, SVR_SOCKET_CONTEXT, m_Overlapped);

		if (_BytesTransferred == 0 && _SocketContext->m_OpType != SVR_OP::SVROP_ACCEPTING) {
			closesocket(_SocketContext->m_ClientSocket);

			delete _SocketContext;
			//TODO
			continue;
		}

		_SocketContext->m_BytesTransferred = _BytesTransferred;

		switch (_SocketContext->m_OpType) {
		case SVR_OP::SVROP_ACCEPTING:
			_WorkerParams->m_Instance->_DoAccepted(_SocketContext);
			break;
		case SVR_OP::SVROP_RECVING:
			_WorkerParams->m_Instance->_DoRecvd(_SocketContext);
			break;
		case SVR_OP::SVROP_SENDING:
			_WorkerParams->m_Instance->_DoSent(_SocketContext);
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

/*-------------------------------------Client Section-------------------------------------*/

network::Client::Client() {
#if(DEBUG&DEBUG_TRACE)
	InitializeCriticalSection(&CRITICAL_PRINT);
#endif

}

network::Client::Client(const ClientConfig &_ClientConfig) {
#if(DEBUG&DEBUG_TRACE)
	InitializeCriticalSection(&CRITICAL_PRINT);
#endif

	m_ClientConfig = _ClientConfig;
}

void network::Client::SetConfig(const ClientConfig &_ClientConfig) {
	m_ClientConfig = _ClientConfig;
}

bool network::Client::Connect() {
	if (_InitCompletionPort()) {
		if (_InitSock()) {
			if (_PostConnect()) {
				return true;
			}
		}
	}

	return false;
}

bool network::Client::Send(const char * _SendBuffer, unsigned int _BufferLen) {
	CLT_SOCKET_CONTEXT *_SocketContext = new CLT_SOCKET_CONTEXT(_SendBuffer, _BufferLen);

	return _PostSend(_SocketContext);
}

bool network::Client::Close() {
	closesocket(m_Socket);

	return true;
}

void network::Client::OnConnected(CLT_SOCKET_CONTEXT * _SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("OnConnected DEBUG_TRACE %u BytesTransferred:%u\n", _SocketContext->_DEBUG_TRACE, _SocketContext->m_BytesTransferred);
#endif
}

void network::Client::OnSent(CLT_SOCKET_CONTEXT * _SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("OnSent DEBUG_TRACE %u BytesTransferred:%u\n", _SocketContext->_DEBUG_TRACE, _SocketContext->m_BytesTransferred);
#endif
}

void network::Client::OnRecvd(CLT_SOCKET_CONTEXT * _SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("OnRecvd DEBUG_TRACE %u BytesTransferred:%u\n", _SocketContext->_DEBUG_TRACE, _SocketContext->m_BytesTransferred);
#endif
}

void network::Client::OnClosed(CLT_SOCKET_CONTEXT * _SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("OnClosed DEBUG_TRACE %u BytesTransferred:%u\n", _SocketContext->_DEBUG_TRACE, _SocketContext->m_BytesTransferred);
#endif
}

bool network::Client::_InitCompletionPort() {
	m_CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (ULONG_PTR)NULL, 0);
	if (m_CompletionPort == NULL) {
		return false;
	}

	unsigned int _WorkerThreadsNum = 1;

	HANDLE* _WorkerThreads = new HANDLE[_WorkerThreadsNum];
	memset(_WorkerThreads, 0, sizeof(HANDLE)*_WorkerThreadsNum);
	DWORD ThreadId;
	for (unsigned int i = 0; i < _WorkerThreadsNum; ++i) {
		WORKER_PARAMS<Client> *_WorkerParams = new WORKER_PARAMS<Client>();
		_WorkerParams->m_Instance = this;
		_WorkerParams->m_ThreadNo = i;

		_WorkerThreads[i] = CreateThread(NULL, 0, ClientWorkThread, _WorkerParams, 0, &ThreadId);
		if (_WorkerThreads[i] == NULL) {
			//TODO CloseHandle
			return false;
		}
	}
	return true;
}

bool network::Client::_InitSock() {
	//WSADATA
	WSADATA Wsadata;
	WORD wVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(wVersionRequested, &Wsadata) != 0) {
		return false;
	}
	if (LOBYTE(Wsadata.wVersion) != 2 || HIBYTE(Wsadata.wVersion) != 2) {
		return false;
	}

	//SOCKET
	m_Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_Socket == INVALID_SOCKET) {
		return false;
	}

	SOCKADDR_IN _LocalAddr;
	memset(&_LocalAddr, 0, sizeof(_LocalAddr));
	_LocalAddr.sin_family = AF_INET;

	//BIND
	if (bind(m_Socket, (SOCKADDR*)&_LocalAddr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
		return false;
	}

	if (CreateIoCompletionPort((HANDLE)m_Socket, m_CompletionPort, (ULONG_PTR)NULL, 0) == NULL) {
		//TODO
		return false;
	}

	//pACCEPTEX pGETACCEPTEXSOCKADDRS
	GUID GuidConnectEx = WSAID_CONNECTEX;
	DWORD dwBytes = 0;
	if (SOCKET_ERROR == WSAIoctl(
		m_Socket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&GuidConnectEx,
		sizeof(GuidConnectEx),
		&m_ConnectEx,
		sizeof(m_ConnectEx),
		&dwBytes,
		NULL,
		NULL)) {
		return false;
	}

	return true;
}

bool network::Client::_PostConnect() {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("PostConnect\n");
#endif

	SOCKADDR_IN _Addr;
	memset(&_Addr, 0, sizeof(SOCKADDR_IN));
	_Addr.sin_family = AF_INET;
	//_Addr.sin_addr.S_un.S_addr = inet_addr("192.168.10.132");
	_Addr.sin_addr.S_un.S_addr = inet_addr("192.168.1.102");
	_Addr.sin_port = htons(80);

	CLT_SOCKET_CONTEXT *_SocketContext = new CLT_SOCKET_CONTEXT();

	_SocketContext->m_OpType = CLT_OP::CLTOP_CONNECTING;

	DWORD dwBytes = 0;
	if (m_ConnectEx(m_Socket, (SOCKADDR*)&_Addr, sizeof(_Addr), NULL, 0, &dwBytes, (LPOVERLAPPED)_SocketContext) == false) {
		if (WSAGetLastError() != ERROR_IO_PENDING) {
			return false;
		}
	}

	return true;
}

bool network::Client::_PostSend(CLT_SOCKET_CONTEXT * _SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("PostSend DEBUG_TRACE %u\n", _SocketContext->_DEBUG_TRACE);
#endif

	DWORD dwBytes = 0;
	DWORD dwFlags = 0;

	_SocketContext->m_OpType = CLT_OP::CLTOP_SENDING;

	if (WSASend(m_Socket, &(_SocketContext->m_wsaBuf), 1, &dwBytes, dwFlags, &(_SocketContext->m_Overlapped), NULL) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING) {
			return false;
		}
	}

	return true;
}

bool network::Client::_PostRecv(CLT_SOCKET_CONTEXT * _SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("PostRecv DEBUG_TRACE %u\n", _SocketContext->_DEBUG_TRACE);
#endif

	DWORD dwBytes = 0;
	DWORD dwFlags = 0;

	_SocketContext->m_OpType = CLT_OP::CLTOP_RECVING;

	if (WSARecv(m_Socket, &(_SocketContext->m_wsaBuf), 1, &dwBytes, &dwFlags, &(_SocketContext->m_Overlapped), NULL) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING) {
			return false;
		}
	}

	return true;
}

bool network::Client::_DoConnected(CLT_SOCKET_CONTEXT * _SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("DoConnected DEBUG_TRACE %u\n", _SocketContext->_DEBUG_TRACE);
#endif

	OnConnected(_SocketContext);

	_SocketContext->RESET_BUFFER();

	_PostRecv(_SocketContext);

	return true;
}

bool network::Client::_DoSent(CLT_SOCKET_CONTEXT * _SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("DoSent DEBUG_TRACE %u\n", _SocketContext->_DEBUG_TRACE);
#endif

	OnSent(_SocketContext);

	delete _SocketContext;//TODO use repeatedly

	return false;
}

bool network::Client::_DoRecvd(CLT_SOCKET_CONTEXT * _SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("DoRecvd DEBUG_TRACE %u\n", _SocketContext->_DEBUG_TRACE);
#endif

	OnRecvd(_SocketContext);

	_PostRecv(_SocketContext);

	return false;
}

DWORD network::Client::ClientWorkThread(LPVOID _LpParam) {
	WORKER_PARAMS<Client> *_WorkerParams = (WORKER_PARAMS<Client>*)_LpParam;

#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("ClientWorkThread ThreadNo:%u\n", _WorkerParams->m_ThreadNo);
#endif

	DWORD _BytesTransferred;
	OVERLAPPED *_Overlapped;
	CLT_SOCKET_CONTEXT *_SocketContext;

	while (true) {
		_BytesTransferred = 0;

		if (GetQueuedCompletionStatus(_WorkerParams->m_Instance->m_CompletionPort, &_BytesTransferred, (PULONG_PTR)&_SocketContext, (LPOVERLAPPED*)&_Overlapped, INFINITE) == false) {
			continue;
		}

		_SocketContext = CONTAINING_RECORD(_Overlapped, CLT_SOCKET_CONTEXT, m_Overlapped);

		if (_BytesTransferred == 0 && _SocketContext->m_OpType != CLT_OP::CLTOP_CONNECTING) {
			closesocket(_WorkerParams->m_Instance->m_Socket);

			delete _SocketContext;
			continue;
		}

		_SocketContext->m_BytesTransferred = _BytesTransferred;

		switch (_SocketContext->m_OpType) {
		case CLT_OP::CLTOP_CONNECTING:
			_WorkerParams->m_Instance->_DoConnected(_SocketContext);
			break;
		case CLT_OP::CLTOP_SENDING:
			_WorkerParams->m_Instance->_DoSent(_SocketContext);
			break;
		case CLT_OP::CLTOP_RECVING:
			_WorkerParams->m_Instance->_DoRecvd(_SocketContext);
			break;
		default:
			break;
		}
	}

	return 0;
}

network::Client::~Client() {
	closesocket(m_Socket);//TODO duplicated with Close()
}

/*-------------------------------------Other Section-------------------------------------*/