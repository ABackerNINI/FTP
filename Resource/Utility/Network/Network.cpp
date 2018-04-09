
#include "Network.h"

/*-----------------------------------------------------------Server Section-----------------------------------------------------------*/

network::Server::Server() {
	//#if(DEBUG&DEBUG_TRACE)
	//	InitializeCriticalSection(&CRITICAL_PRINT);
	//#endif
}

network::Server::Server(const ServerConfig &_ServerConfig) {
	m_ServerConfig = _ServerConfig;
}

void network::Server::SetConfig(const ServerConfig &_ServerConfig) {
	m_ServerConfig = _ServerConfig;
}

bool network::Server::Start() {
	return _Start(m_ServerConfig.M_Port, m_ServerConfig.O_MaxConnect);
}

bool network::Server::Send(SOCKET _Socket, const char * _SendBuffer, size_t _BufferLen) {
	SVR_SOCKET_CONTEXT *_SocketContext = new SVR_SOCKET_CONTEXT(_Socket, _SendBuffer, _BufferLen);

	return _PostSend(_SocketContext);
}

bool network::Server::Close(SOCKET _Socket) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("Close Socket:%lld @Close\n", _Socket);
#endif

	closesocket(_Socket);

	return true;
}

//bool network::Server::AddListenPort(int _Port, int _Max_Connect = 1) {
//	return _AddListenPort(_Port, _Max_Connect);
//}

bool network::Server::Stop() {
	_Stop(m_Socket);

	return true;
}

void network::Server::OnAccepted(SVR_SOCKET_CONTEXT * _SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("OnAccepted DEBUG_TRACE %u BytesTransferred:%u @OnAccepted\n", _SocketContext->_DEBUG_TRACE, _SocketContext->m_BytesTransferred);
#endif

	if (_SocketContext->m_BytesTransferred) {
		printf("%s\n", _SocketContext->m_szBuffer);
		//Send(_SocketContext->m_ClientSocket, _SocketContext->m_szBuffer, _SocketContext->m_BytesTransferred);
	}
}

void network::Server::OnRecvd(SVR_SOCKET_CONTEXT * _SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("OnRecvd DEBUG_TRACE %u BytesTransferred:%u @OnRecvd\n", _SocketContext->_DEBUG_TRACE, _SocketContext->m_BytesTransferred);
#endif

	printf("%s\n", _SocketContext->m_szBuffer);

	//if (_SocketContext->m_BytesTransferred) {
	//	Send(_SocketContext->m_ClientSocket, _SocketContext->m_szBuffer, _SocketContext->m_BytesTransferred);
	//}
}

void network::Server::OnSent(SVR_SOCKET_CONTEXT * _SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("OnSent DEBUG_TRACE %u BytesTransferred:%u @OnSent\n", _SocketContext->_DEBUG_TRACE, _SocketContext->m_BytesTransferred);
#endif
}

void network::Server::OnClosed(SVR_SOCKET_CONTEXT * _SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("OnClosed DEBUG_TRACE %u BytesTransferred:%u @OnClosed\n", _SocketContext->_DEBUG_TRACE, _SocketContext->m_BytesTransferred);
#endif
}

bool network::Server::_InitComplitionPort() {
	//Completion Port
	m_CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_CompletionPort == NULL) {
#if(DEBUG&DEBUG_LOG)
		LOG(CC_RED, "Faild to Create ComplitionPort @_InitComplitionPort\n");
#endif
		return false;
	}

	unsigned int _WorkerThreadsNum = m_ServerConfig.O0_WorkerThreads > 0 ? m_ServerConfig.O0_WorkerThreads : (_GetProcessorNum()*m_ServerConfig.O0_WorkerThreadsPerProcessor);
	if (_WorkerThreadsNum <= 0) {
#if(DEBUG&DEBUG_LOG)
		LOG(CC_RED, "Invalid Worker Threads Number @_InitComplitionPort\n");
#endif
		return false;
	}

	HANDLE* _WorkerThreads = new HANDLE[_WorkerThreadsNum];
	memset(_WorkerThreads, 0, sizeof(HANDLE)*_WorkerThreadsNum);
	DWORD ThreadId;
	for (unsigned int i = 0; i < _WorkerThreadsNum; ++i) {
		WORKER_PARAMS<Server> *_pWorkerParams = new WORKER_PARAMS<Server>();
		_pWorkerParams->m_Instance = this;
		_pWorkerParams->m_ThreadNo = i;

		_WorkerThreads[i] = CreateThread(NULL, 0, ServerWorkThread, _pWorkerParams, 0, &ThreadId);
		if (_WorkerThreads[i] == NULL) {
#if(DEBUG&DEBUG_LOG)
			LOG(CC_RED, "Failed to Start Thread%u @_InitComplitionPort\n", i);
#endif
		}
	}

	return true;
}

//bool network::Server::_AddListenPort(int _Port, int _Max_Connect) {
//	//SOCKET
//	m_Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
//	if (m_Socket == INVALID_SOCKET) {
//#if(DEBUG&DEBUG_LOG)
//		LOG(CC_RED, "Faild to Create Socket @_AddListenPort\n");
//#endif
//		return false;
//	}
//
//	SOCKADDR_IN _Addr;
//	memset(&_Addr, 0, sizeof(_Addr));
//	_Addr.sin_family = AF_INET;
//	_Addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
//	_Addr.sin_port = htons(_Port);
//
//	//BIND
//	if (bind(m_Socket, (SOCKADDR*)&_Addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
//#if(DEBUG&DEBUG_LOG)
//		LOG(CC_RED, "Faild to Bind Socket @_AddListenPort\n");
//#endif
//		return false;
//	}
//
//	//LISTEN
//	if (listen(m_Socket, _Max_Connect) == SOCKET_ERROR) {
//#if(DEBUG&DEBUG_LOG)
//		LOG(CC_RED, "Faild to Listen Port %d @_AddListenPort\n", _Port);
//#endif
//		return false;
//	}
//
//	//COMPLETIONPORT
//	if (CreateIoCompletionPort((HANDLE)m_Socket, m_CompletionPort, (ULONG_PTR)NULL, 0) == NULL) {
//#if(DEBUG&DEBUG_LOG)
//		LOG(CC_RED, "Faild to Bind Socket with CompletionPort @_AddListenPort\n");
//#endif
//		return false;
//	}
//
//	//POST ACCEPT
//	for (int i = 0; i < m_ServerConfig.O_MaxPostAccept; ++i) {
//		SVR_SOCKET_CONTEXT *_SocketContext = new SVR_SOCKET_CONTEXT();
//		if (_PostAccept(_SocketContext) == false) {
//#if(DEBUG&DEBUG_LOG)
//			LOG(CC_RED, "Faild to Post Accept%d @_AddListenPort\n", i);
//#endif
//		}
//	}
//
//	return true;
//}

bool network::Server::_InitSock(int _Port, unsigned int _Max_Connect) {
	//WSADATA
	WSADATA Wsadata;
	WORD wVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(wVersionRequested, &Wsadata) != 0) {
#if(DEBUG&DEBUG_LOG)
		LOG(CC_RED, "Faild to Load WSAStartup @_InitSock\n");
#endif
		return false;
	}

	if (LOBYTE(Wsadata.wVersion) != 2 || HIBYTE(Wsadata.wVersion) != 2) {
#if(DEBUG&DEBUG_LOG)
		LOG(CC_RED, "Wrong WSA Version(!2.2) @_InitSock\n");
#endif
		return false;
	}

	//SOCKET
	m_Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_Socket == INVALID_SOCKET) {
#if(DEBUG&DEBUG_LOG)
		LOG(CC_RED, "Faild to Create Socket @_InitSock\n");
#endif
		return false;
	}

	SOCKADDR_IN _Addr;
	memset(&_Addr, 0, sizeof(_Addr));
	_Addr.sin_family = AF_INET;
	_Addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	_Addr.sin_port = htons(_Port);

	//BIND
	if (bind(m_Socket, (SOCKADDR*)&_Addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
#if(DEBUG&DEBUG_LOG)
		LOG(CC_RED, "Faild to Bind Socket @_InitSock\n");
#endif
		return false;
	}

	//LISTEN
	if (listen(m_Socket, _Max_Connect) == SOCKET_ERROR) {
#if(DEBUG&DEBUG_LOG)
		LOG(CC_RED, "Faild to Listen Port %d @_InitSock\n", _Port);
#endif
		return false;
	}

	//COMPLETIONPORT
	if (CreateIoCompletionPort((HANDLE)m_Socket, m_CompletionPort, (ULONG_PTR)NULL, 0) == NULL) {
#if(DEBUG&DEBUG_LOG)
		LOG(CC_RED, "Faild to Bind Socket with CompletionPort @_InitSock\n");
#endif
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
#if(DEBUG&DEBUG_LOG)
		LOG(CC_RED, "Faild to Get AcceptEx @_InitSock\n");
#endif
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
#if(DEBUG&DEBUG_LOG)
		LOG(CC_RED, "Faild to Get GetAcceptExSockAddrs @_InitSock\n");
#endif
		return false;
	}

	//POST ACCEPT
	for (int i = 0; i < m_ServerConfig.O_MaxPostAccept; ++i) {
		SVR_SOCKET_CONTEXT *_SocketContext = new SVR_SOCKET_CONTEXT();
		if (_PostAccept(_SocketContext) == false) {
#if(DEBUG&DEBUG_LOG)
			LOG(CC_RED, "Faild to Post Accept%d @_InitSock\n", i);
#endif
		}
	}

	return true;
}

bool network::Server::_Start(int _Port, int _MaxConnect = SOMAXCONN) {
	if (_InitComplitionPort() == false) {
#if(DEBUG&DEBUG_LOG)
		LOG(CC_RED, "Faild to Init ComplitionPort @_Start\n");
#endif
		return false;
	}

	if (_InitSock(_Port, _MaxConnect) == false) {
#if(DEBUG&DEBUG_LOG)
		LOG(CC_RED, "Faild to Init Sock @_Start\n");
#endif
		return false;
	}

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
#if(FEATURE_RECV_ON_ACCEPT)
		_SocketContext->m_wsaBuf.len - ((sizeof(SOCKADDR_IN) + 16) * 2),
#else
		0,
#endif
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		&_Flags,
		&(_SocketContext->m_Overlapped)) == false) {
		if (WSAGetLastError() != WSA_IO_PENDING) {
#if(DEBUG&DEBUG_LOG)
			LOG(CC_RED, "Faild to Post Accept Socket:%lld @PostAccept\n", _SocketContext->m_ClientSocket);
#endif
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
#if(DEBUG&DEBUG_LOG)
		LOG(CC_RED, "Faild to Post Recv Socket:%lld @_PostRecv\n", _SocketContext->m_ClientSocket);
#endif
		return false;
	}

	return true;
}

bool network::Server::_PostSend(SVR_SOCKET_CONTEXT *_SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("PostSend DEBUG_TRACE %u\n", _SocketContext->_DEBUG_TRACE);
#endif

	if (_SocketContext->m_wsaBuf.len == 0) {
		return true;
	}

	DWORD dwFlags = 0;
	DWORD dwBytes = 0;

	_SocketContext->m_OpType = SVR_OP::SVROP_SENDING;

	if (WSASend(_SocketContext->m_ClientSocket, &(_SocketContext->m_wsaBuf), 1, &dwBytes, dwFlags, &(_SocketContext->m_Overlapped), NULL) == SOCKET_ERROR &&
		WSAGetLastError() != WSA_IO_PENDING) {
#if(DEBUG&DEBUG_LOG)
		LOG(CC_RED, "Faild to Post Send Socket:%lld @_PostSend\n", _SocketContext->m_ClientSocket);
#endif
		return false;
	}

	return true;
}

bool network::Server::_DoAccepted(SVR_SOCKET_CONTEXT *_SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("DoAccept DEBUG_TRACE %u Socket:%d @_DoAccepted\n", _SocketContext->_DEBUG_TRACE, _SocketContext->m_ClientSocket);
#endif

#if(DEBUG&DEBUG_LOG)
	LOG(CC_YELLOW, "Accepted a Client Socket:%lld @_DoAccepted\n", _SocketContext->m_ClientSocket);
#endif

	_SocketContext->m_szBuffer[_SocketContext->m_BytesTransferred] = '\0';

	OnAccepted(_SocketContext);

	_SocketContext->RESET_BUFFER();

	SVR_SOCKET_CONTEXT *_NewSocketContex = new SVR_SOCKET_CONTEXT();
	_NewSocketContex->m_OpType = SVR_OP::SVROP_RECVING;
	_NewSocketContex->m_ClientSocket = _SocketContext->m_ClientSocket;
	_NewSocketContex->m_Extra = _SocketContext->m_Extra;
	SOCKADDR_IN *_ClientAddr, *_LocalAddr;//mark:need to delete?
	int _ClientAddrLen = sizeof(SOCKADDR_IN), _LocalAddrLen = sizeof(SOCKADDR_IN);

	m_pGetAcceptExSockAddrs(
		_NewSocketContex->m_wsaBuf.buf,
#if(FEATURE_RECV_ON_ACCEPT)
        _NewSocketContex->m_wsaBuf.len - ((sizeof(SOCKADDR_IN) + 16) * 2),
#else
        0,
#endif
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
		(LPSOCKADDR*)&_LocalAddr, 
		&_LocalAddrLen,
		(LPSOCKADDR*)&_ClientAddr,
		&_ClientAddrLen);//mark:?

	_NewSocketContex->m_ClientAddr = *_ClientAddr;//mark

	//TODO save clientaddr

	if (CreateIoCompletionPort((HANDLE)_NewSocketContex->m_ClientSocket, m_CompletionPort, (ULONG_PTR)_NewSocketContex, 0) == NULL) {
#if(DEBUG&DEBUG_LOG)
		LOG(CC_RED, "Faild to Bind Socket with CompletionPort Socket:%lld @_DoAccepted\n", _SocketContext->m_ClientSocket);
#endif
		return false;
	}

	if (_PostRecv(_NewSocketContex) == false) {
#if(DEBUG&DEBUG_LOG)
		LOG(CC_RED, "Faild to Post Recv Socket:%lld @_DoAccepted\n", _SocketContext->m_ClientSocket);
#endif
		return false;
	}

	if (_PostAccept(_SocketContext) == false) {
#if(DEBUG&DEBUG_LOG)
		LOG(CC_RED, "Faild to Post Accept Socket:%lld @_DoAccepted\n", _SocketContext->m_ClientSocket);
#endif
		return false;
	}

	return true;
}

bool network::Server::_DoRecvd(SVR_SOCKET_CONTEXT *_SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("DoRecv DEBUG_TRACE %u @_DoRecvd\n", _SocketContext->_DEBUG_TRACE);
#endif

	_SocketContext->m_szBuffer[_SocketContext->m_BytesTransferred] = '\0';

	OnRecvd(_SocketContext);

	_SocketContext->RESET_BUFFER();

	_SocketContext->m_OpType = SVR_OP::SVROP_RECVING;

	if (_PostRecv(_SocketContext) == false) {
#if(DEBUG&DEBUG_LOG)
		LOG(CC_RED, "Faild to Post Recv Socket:%lld @_DoRecvd\n", _SocketContext->m_ClientSocket);
#endif
		return false;
	}

	return true;
}

bool network::Server::_DoSent(SVR_SOCKET_CONTEXT *_SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("DoSend DEBUG_TRACE %u @_DoSent\n", _SocketContext->_DEBUG_TRACE);
#endif

	OnSent(_SocketContext);

	delete _SocketContext;//TODO use repeatedly

	return true;
}

bool network::Server::_IsClientAlive(SOCKET _Sockid) {
    int nByteSent = send(_Sockid, "", 0, 0);

    return nByteSent != -1;
}

DWORD WINAPI network::Server::ServerWorkThread(LPVOID _LpParam) {
	WORKER_PARAMS<Server> *_WorkerParams = (WORKER_PARAMS<Server>*)_LpParam;

#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("ServerWorkThread ThreadNo:%u @ServerWorkThread\n", _WorkerParams->m_ThreadNo);
#endif

	DWORD _BytesTransferred;
	OVERLAPPED *_Overlapped;
	SVR_SOCKET_CONTEXT *_SocketContext;
	HANDLE _CompletionPort = _WorkerParams->m_Instance->m_CompletionPort;
	Server *_Server = _WorkerParams->m_Instance;

	typedef bool(*DoEvent)(Server*, SVR_SOCKET_CONTEXT*);
	typedef void(*OnEvent)(Server*, SVR_SOCKET_CONTEXT*);

	DoEvent _DoAccepted = pointer_cast<DoEvent>(&Server::_DoAccepted);
	DoEvent _DoRecvd = pointer_cast<DoEvent>(&Server::_DoRecvd);
	DoEvent _DoSent = pointer_cast<DoEvent>(&Server::_DoSent);

	OnEvent _OnClosed = pointer_cast<OnEvent>(&Server::OnClosed);

	BOOL _Ret;
    DWORD _ErrCode;

	while (true) {
		_BytesTransferred = 0;

		_Ret = GetQueuedCompletionStatus(_CompletionPort, &_BytesTransferred, (PULONG_PTR)&_SocketContext, (LPOVERLAPPED*)&_Overlapped, INFINITE);

        if (_Ret == false/* && _Overlapped == NULL*/) {
#if(DEBUG&DEBUG_LOG)
            LOG(CC_RED, "GetQueuedCompletionStatus Failed @ServerWorkThread\n");
#endif

            _ErrCode = GetLastError();

            if (_ErrCode == WAIT_TIMEOUT) {
                if (!_IsClientAlive(_SocketContext->m_ClientSocket)) {
#if(DEBUG&DEBUG_LOG)
                    LOG(CC_YELLOW, "Client Error Socket:%lld @ServerWorkThread\n", _SocketContext->m_ClientSocket);
#endif
                    _OnClosed(_Server, _SocketContext);

                    closesocket(_SocketContext->m_ClientSocket);

                    delete _SocketContext;
                }
                continue;
            } else if (_ErrCode == ERROR_NETNAME_DELETED) {
                if (_SocketContext) {
#if(DEBUG&DEBUG_LOG)
                    LOG(CC_YELLOW, "Client Error Socket:%lld @ServerWorkThread\n", _SocketContext->m_ClientSocket);
#endif
                    _OnClosed(_Server, _SocketContext);

                    closesocket(_SocketContext->m_ClientSocket);

                    delete _SocketContext;
                } else {
                    // TODO
                }
            }

            continue;
        }

		_SocketContext = CONTAINING_RECORD(_Overlapped, SVR_SOCKET_CONTEXT, m_Overlapped);

        if (_BytesTransferred == 0 && _SocketContext->m_OpType != SVR_OP::SVROP_ACCEPTING) {
#if(DEBUG&DEBUG_LOG)
            LOG(CC_YELLOW, "Client Offline Socket:%lld @ServerWorkThread\n", _SocketContext->m_ClientSocket);
#endif
            _OnClosed(_Server, _SocketContext);

            closesocket(_SocketContext->m_ClientSocket);

            delete _SocketContext;
        }

		_SocketContext->m_BytesTransferred = _BytesTransferred;

		switch (_SocketContext->m_OpType) {
		case SVR_OP::SVROP_ACCEPTING:
			_DoAccepted(_Server, _SocketContext);
			break;
		case SVR_OP::SVROP_RECVING:
			_DoRecvd(_Server, _SocketContext);
			break;
		case SVR_OP::SVROP_SENDING:
			_DoSent(_Server, _SocketContext);
			break;
		default:
			break;
		}
	}

	return 0;
}

/*-----------------------------------------------------------Client Section-----------------------------------------------------------*/

network::Client::Client() :m_CompletionPort(NULL) {
	_Init();
}

network::Client::Client(const ClientConfig &_ClientConfig) : m_CompletionPort(NULL) {
	m_ClientConfig = _ClientConfig;

	_Init();
}

void network::Client::SetConfig(const ClientConfig &_ClientConfig) {
	m_ClientConfig = _ClientConfig;
}

int network::Client::_Init() {
	//WSADATA
	WSADATA Wsadata;
	WORD wVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(wVersionRequested, &Wsadata) != 0) {
#if(DEBUG&DEBUG_LOG)
		LOG(CC_RED, "Faild to Load WSAStartup @_Init\n");
#endif
		return -1;
	}

	if (LOBYTE(Wsadata.wVersion) != 2 || HIBYTE(Wsadata.wVersion) != 2) {
#if(DEBUG&DEBUG_LOG)
		LOG(CC_RED, "Wrong WSA Version(!2.2) @_Init\n");
#endif
		return -2;
	}

	if (_InitCompletionPort() == false) {
#if(DEBUG&DEBUG_LOG)
		LOG(CC_RED, "Faild to Init ComplitionPort @_Init\n");
#endif
		return -3;
	}

	return 0;
}

SOCKET network::Client::Connect(const IP_PORT * _IpPort, int *_LocalPort) {
	int _LocalPort1 = 0;
	if (!_LocalPort) {
		_LocalPort = &_LocalPort1;
	}

	SOCKET _Socket = _InitSock(_LocalPort);
	if (*_LocalPort < 0) {
#if(DEBUG&DEBUG_LOG)
		LOG(CC_RED, "Faild to Init Sock @Connect\n");
#endif
		return -2;
	}

	if (_PostConnect(_Socket, _IpPort->M0_Ip_ULong ? _IpPort->M0_Ip_ULong : inet_addr(_IpPort->M0_Ip_String), _IpPort->M_Port) == false) {
#if(DEBUG&DEBUG_LOG)
		LOG(CC_RED, "Faild to Post Connect @Connect\n");
#endif
		return -3;
	}

	return _Socket;
}

SOCKET network::Client::Connect(const char * _Address, int * _LocalPort) {
	IP_PORT _IpPort;

	//TODO parse address and call gethostbyname

	return Connect(&_IpPort, _LocalPort);
}

bool network::Client::Send(SOCKET _Socket, const char * _SendBuffer, size_t _BufferLen) {
	CLT_SOCKET_CONTEXT *_SocketContext = new CLT_SOCKET_CONTEXT(_SendBuffer, _BufferLen);//"port 1234\r\n"
	_SocketContext->m_Socket = _Socket;

	return _PostSend(_SocketContext);
}

bool network::Client::Close(SOCKET _Socket) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("Close Socket:%lld @Close\n", _Socket);
#endif

	//shutdown(m_Socket, SD_BOTH);

	//LINGER _Linger = { 1,0 };
	//_Linger.l_onoff = 0;
	//setsockopt(m_Socket, SOL_SOCKET, SO_LINGER, (const char *)&_Linger, sizeof(_Linger));

	closesocket(_Socket);

	return true;
}

void network::Client::OnConnected(CLT_SOCKET_CONTEXT * _SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("OnConnected DEBUG_TRACE %u BytesTransferred:%u @OnConnected\n", _SocketContext->_DEBUG_TRACE, _SocketContext->m_BytesTransferred);
#endif

	if (_SocketContext->m_BytesTransferred) {
		printf("%s\n", _SocketContext->m_szBuffer);
	}
}

void network::Client::OnSent(CLT_SOCKET_CONTEXT * _SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("OnSent DEBUG_TRACE %u BytesTransferred:%u @OnSent\n", _SocketContext->_DEBUG_TRACE, _SocketContext->m_BytesTransferred);
#endif
}

void network::Client::OnRecvd(CLT_SOCKET_CONTEXT * _SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("OnRecvd DEBUG_TRACE %u BytesTransferred:%u @OnRecvd\n", _SocketContext->_DEBUG_TRACE, _SocketContext->m_BytesTransferred);
#endif

	printf("%s\n", _SocketContext->m_szBuffer);
}

void network::Client::OnClosed(CLT_SOCKET_CONTEXT * _SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("OnClosed DEBUG_TRACE %u BytesTransferred:%u @OnClosed\n", _SocketContext->_DEBUG_TRACE, _SocketContext->m_BytesTransferred);
#endif
}

bool network::Client::_InitCompletionPort() {
	m_CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (ULONG_PTR)NULL, 0);
	if (m_CompletionPort == NULL) {
#if(DEBUG&DEBUG_LOG)
		LOG(CC_RED, "Faild to Create CompletionPort @_InitCompletionPort\n");
#endif
		return false;
	}

	unsigned int _WorkerThreadsNum = m_ClientConfig.O0_WorkerThreads > 0 ? m_ClientConfig.O0_WorkerThreads : (m_ClientConfig.O0_WorkerThreadsPerProcessor * _GetProcessorNum());
	if (_WorkerThreadsNum <= 0) {
#if(DEBUG&DEBUG_LOG)
		LOG(CC_RED, "Invalid Worker Threads Number @_InitCompletionPort\n");
#endif
		return false;
	}

	HANDLE* _WorkerThreads = new HANDLE[_WorkerThreadsNum];
	memset(_WorkerThreads, 0, sizeof(HANDLE)*_WorkerThreadsNum);
	DWORD ThreadId;
	for (unsigned int i = 0; i < _WorkerThreadsNum; ++i) {
		WORKER_PARAMS<Client> *_WorkerParams = new WORKER_PARAMS<Client>();
		_WorkerParams->m_Instance = this;
		_WorkerParams->m_ThreadNo = i;

		_WorkerThreads[i] = CreateThread(NULL, 0, ClientWorkThread, _WorkerParams, 0, &ThreadId);
		if (_WorkerThreads[i] == NULL) {
#if(DEBUG&DEBUG_LOG)
			LOG(CC_RED, "Faild to Start Thread%u @_InitCompletionPort\n", i);
#endif
		}
	}
	return true;
}

SOCKET network::Client::_InitSock(int *_Port) {

	//SOCKET
	SOCKET _Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (_Socket == INVALID_SOCKET) {
#if(DEBUG&DEBUG_LOG)
		LOG(CC_RED, "Faild to Create Socket @_InitSock\n");
#endif
		return -3;
	}

	SOCKADDR_IN _LocalAddr;
	memset(&_LocalAddr, 0, sizeof(_LocalAddr));
	_LocalAddr.sin_family = AF_INET;
	_LocalAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	_LocalAddr.sin_port = htons(_Port ? (short)*_Port : 0);

	//BIND
	if (bind(_Socket, (SOCKADDR*)&_LocalAddr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
#if(DEBUG&DEBUG_LOG)
		LOG(CC_RED, "Faild to Bind Socket @_InitSock\n");
#endif
		return -4;
	}

	if (CreateIoCompletionPort((HANDLE)_Socket, m_CompletionPort, (ULONG_PTR)NULL, 0) == NULL) {
#if(DEBUG&DEBUG_LOG)
		LOG(CC_RED, "Faild to Bind Socket with CompletionPort @_InitSock\n");
#endif
		return -5;
	}

	if (!m_ConnectEx) {
		//pACCEPTEX pGETACCEPTEXSOCKADDRS
		GUID GuidConnectEx = WSAID_CONNECTEX;
		DWORD dwBytes = 0;
		if (SOCKET_ERROR == WSAIoctl(
			_Socket,
			SIO_GET_EXTENSION_FUNCTION_POINTER,
			&GuidConnectEx,
			sizeof(GuidConnectEx),
			&m_ConnectEx,
			sizeof(m_ConnectEx),
			&dwBytes,
			NULL,
			NULL)) {
#if(DEBUG&DEBUG_LOG)
			LOG(CC_RED, "Faild to Get ConnectEx @_InitSock\n");
#endif
			return -6;
		}
	}

	*_Port = _LocalAddr.sin_port;

	return _Socket;
}

bool network::Client::_PostConnect(SOCKET _Socket, unsigned long _Ip, int _Port) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("PostConnect @_PostConnect\n");
#endif

	SOCKADDR_IN _Addr;
	memset(&_Addr, 0, sizeof(SOCKADDR_IN));
	_Addr.sin_family = AF_INET;
	_Addr.sin_addr.S_un.S_addr = _Ip;
	_Addr.sin_port = htons(_Port);

	CLT_SOCKET_CONTEXT *_SocketContext = new CLT_SOCKET_CONTEXT();

	_SocketContext->m_Socket = _Socket;
	_SocketContext->m_OpType = CLT_OP::CLTOP_CONNECTING;

	DWORD dwBytes = 0;
	if (m_ConnectEx(_Socket, (SOCKADDR*)&_Addr, sizeof(_Addr), NULL, 0, &dwBytes, (LPOVERLAPPED)_SocketContext) == false) {
		if (WSAGetLastError() != ERROR_IO_PENDING) {
#if(DEBUG&DEBUG_LOG)
			LOG(CC_RED, "Faild to Post Connect%d @_PostConnect\n");
#endif
			return false;
		}
	}

	return true;
}

bool network::Client::_PostSend(CLT_SOCKET_CONTEXT * _SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("PostSend DEBUG_TRACE %u @_PostSend\n", _SocketContext->_DEBUG_TRACE);
#endif

	/*if (_SocketContext->m_wsaBuf.len == 0) {
		return true;
	}*/

	DWORD dwFlags = 0;
	DWORD dwBytes = 0;

	_SocketContext->m_OpType = CLT_OP::CLTOP_SENDING;

	if (WSASend(_SocketContext->m_Socket, &(_SocketContext->m_wsaBuf), 1, &dwBytes, dwFlags, &(_SocketContext->m_Overlapped), NULL) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING) {
#if(DEBUG&DEBUG_LOG)
			LOG(CC_RED, "Faild to Post Send %d @_PostSend\n", WSAGetLastError());
#endif
			return false;
		}
	}

	return true;
}

bool network::Client::_PostRecv(CLT_SOCKET_CONTEXT * _SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("PostRecv DEBUG_TRACE %u @_PostRecv\n", _SocketContext->_DEBUG_TRACE);
#endif

	DWORD dwFlags = 0;
	DWORD dwBytes = 0;

	_SocketContext->m_OpType = CLT_OP::CLTOP_RECVING;

	if (WSARecv(_SocketContext->m_Socket, &(_SocketContext->m_wsaBuf), 1, &dwBytes, &dwFlags, &(_SocketContext->m_Overlapped), NULL) == SOCKET_ERROR) {
		if (WSAGetLastError() != WSA_IO_PENDING) {
#if(DEBUG&DEBUG_LOG)
			LOG(CC_RED, "Faild to Post Recv @_PostRecv\n");
#endif
			return false;
		}
	}

	return true;
}

bool network::Client::_DoConnected(CLT_SOCKET_CONTEXT * _SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("DoConnected DEBUG_TRACE %u @_DoConnected\n", _SocketContext->_DEBUG_TRACE);
#endif

	_SocketContext->m_szBuffer[_SocketContext->m_BytesTransferred] = '\0';

	OnConnected(_SocketContext);

	_SocketContext->RESET_BUFFER();

	if (_PostRecv(_SocketContext) == false) {
#if(DEBUG&DEBUG_LOG)
		LOG(CC_RED, "Faild to Post Recv @_DoConnected\n");
#endif
		return false;
	}

	return true;
}

bool network::Client::_DoSent(CLT_SOCKET_CONTEXT * _SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("DoSent DEBUG_TRACE %u @_DoSent\n", _SocketContext->_DEBUG_TRACE);
#endif

	OnSent(_SocketContext);

	delete _SocketContext;//TODO use repeatedly

	return false;
}

bool network::Client::_DoRecvd(CLT_SOCKET_CONTEXT * _SocketContext) {
#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("DoRecvd DEBUG_TRACE %u @_DoRecvd\n", _SocketContext->_DEBUG_TRACE);
#endif

	_SocketContext->m_szBuffer[_SocketContext->m_BytesTransferred] = '\0';

	OnRecvd(_SocketContext);

	_PostRecv(_SocketContext);

	return false;
}

DWORD network::Client::ClientWorkThread(LPVOID _LpParam) {
	WORKER_PARAMS<Client> *_WorkerParams = (WORKER_PARAMS<Client>*)_LpParam;

#if(DEBUG&DEBUG_TRACE)
	TRACE_PRINT("ClientWorkThread ThreadNo:%u @ClientWorkThread\n", _WorkerParams->m_ThreadNo);
#endif

	DWORD _BytesTransferred;
	OVERLAPPED *_Overlapped;
	CLT_SOCKET_CONTEXT *_SocketContext;
	HANDLE _CompletionPort = _WorkerParams->m_Instance->m_CompletionPort;
	Client *_Client = _WorkerParams->m_Instance;

	typedef void(*DoEvent)(Client*, CLT_SOCKET_CONTEXT*);
	typedef void(*OnEvent)(Client*, CLT_SOCKET_CONTEXT*);

	DoEvent _DoConnected = pointer_cast<DoEvent>(&(Client::_DoConnected));
	DoEvent _DoSent = pointer_cast<DoEvent>(&(Client::_DoSent));
	DoEvent _DoRecvd = pointer_cast<DoEvent>(&(Client::_DoRecvd));

	OnEvent _OnClosed = pointer_cast<OnEvent>(&Client::OnClosed);

	BOOL _Ret;

    DWORD _ErrCode;

	while (true) {
		_BytesTransferred = 0;

		_Ret = GetQueuedCompletionStatus(_CompletionPort, &_BytesTransferred, (PULONG_PTR)&_SocketContext, (LPOVERLAPPED*)&_Overlapped, INFINITE);

        if (_Ret == false) {
#if(DEBUG&DEBUG_LOG)
            LOG(CC_RED, "GetQueuedCompletionStatus Failed @ClientWorkThread\n");
#endif

            _ErrCode = GetLastError();

            if (_ErrCode == WAIT_TIMEOUT) {
                if (!_IsServerAlive(_SocketContext->m_Socket)) {
#if(DEBUG&DEBUG_LOG)
                    LOG(CC_YELLOW, "Server Error Socket:%lld @ClientWorkThread\n", _SocketContext->m_Socket);
#endif
                    _OnClosed(_Client, _SocketContext);

                    closesocket(_SocketContext->m_Socket);

                    delete _SocketContext;
                }
                continue;
            } else if (_ErrCode == ERROR_NETNAME_DELETED) {
                if (_SocketContext) {
#if(DEBUG&DEBUG_LOG)
                    LOG(CC_YELLOW, "Server Error Socket:%lld @ClientWorkThread\n", _SocketContext->m_Socket);
#endif
                    _OnClosed(_Client, _SocketContext);

                    closesocket(_SocketContext->m_Socket);

                    delete _SocketContext;
                }

                continue;
            } else {
                // TODO
            }
        }

		_SocketContext = CONTAINING_RECORD(_Overlapped, CLT_SOCKET_CONTEXT, m_Overlapped);

		if (_BytesTransferred == 0 && _SocketContext->m_OpType != CLT_OP::CLTOP_CONNECTING) {
#if(DEBUG&DEBUG_LOG)
			LOG(CC_YELLOW, "Server Offline Socket:%lld @ClientWorkThread\n", _SocketContext->m_Socket);
#endif
			_OnClosed(_Client, _SocketContext);

			closesocket(_SocketContext->m_Socket);

			delete _SocketContext;

			continue;
		}

		_SocketContext->m_BytesTransferred = _BytesTransferred;

		switch (_SocketContext->m_OpType) {
		case CLT_OP::CLTOP_CONNECTING:
			_DoConnected(_Client, _SocketContext);
			break;
		case CLT_OP::CLTOP_SENDING:
			_DoSent(_Client, _SocketContext);
			break;
		case CLT_OP::CLTOP_RECVING:
			_DoRecvd(_Client, _SocketContext);
			break;
		default:
			break;
		}
	}

	return 0;
}

bool network::Client::_IsServerAlive(SOCKET _Sockid) {
    return true;
}

network::Client::~Client() {
	//closesocket(m_Socket);//TODO duplicated with Close()
}