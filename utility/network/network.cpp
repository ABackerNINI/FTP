
#include "network.h"

/*
 * Cleanup
 */
int network::Cleanup() {
    return WSACleanup();
}

/*
 * SVR_SOCKET_CONTEXT
 */
network::SVR_SOCKET_CONTEXT::SVR_SOCKET_CONTEXT(size_t _MaxBufferLen/* = DEFAULT_MAX_BUFFER_LEN*/) :
    m_BytesTransferred(0),
    m_OpType(SVR_OP::SVROP_NULL),
    m_ClientSockid(INVALID_SOCKET),
    m_Extra(NULL) {
    m_szBuffer = new char[_MaxBufferLen];//TODO user-defined(upper layer) buffer len
    m_wsaBuf.buf = m_szBuffer;
    m_wsaBuf.len = (ULONG)_MaxBufferLen;

    memset(&m_Overlapped, 0, sizeof(OVERLAPPED));

#if(DEBUG&DEBUG_TRACE)
    _DEBUG_TRACE = network::_DEBUG_TRACE++;
#endif
}

network::SVR_SOCKET_CONTEXT::SVR_SOCKET_CONTEXT(SOCKET _Sockid, char *_Buffer, size_t _BufferLen) :
    m_BytesTransferred(0),
    m_OpType(SVR_OP::SVROP_NULL),
    m_ClientSockid(_Sockid),
    m_Extra(NULL) {
    m_szBuffer = _Buffer;
    m_wsaBuf.buf = m_szBuffer;
    m_wsaBuf.len = (ULONG)_BufferLen;

    memset(&m_Overlapped, 0, sizeof(OVERLAPPED));

#if(DEBUG&DEBUG_TRACE)
    _DEBUG_TRACE = network::_DEBUG_TRACE++;
#endif
}

network::SVR_SOCKET_CONTEXT::SVR_SOCKET_CONTEXT(SOCKET _Sockid, const char *_Buffer, size_t _BufferLen) :
    m_BytesTransferred(0),
    m_OpType(SVR_OP::SVROP_NULL),
    m_ClientSockid(_Sockid),
    m_Extra(NULL) {
    m_szBuffer = new char[_BufferLen];
    memcpy(m_szBuffer, _Buffer, sizeof(char)*_BufferLen);
    m_wsaBuf.buf = m_szBuffer;
    m_wsaBuf.len = (ULONG)_BufferLen;

    memset(&m_Overlapped, 0, sizeof(OVERLAPPED));

#if(DEBUG&DEBUG_TRACE)
    _DEBUG_TRACE = network::_DEBUG_TRACE++;
#endif
}

void network::SVR_SOCKET_CONTEXT::RESET_BUFFER() {
    m_BytesTransferred = 0;
}

network::SVR_SOCKET_CONTEXT::~SVR_SOCKET_CONTEXT() {
    if (m_szBuffer)
        delete[] m_szBuffer;

#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("PSC Dispose DEBUG_TRACE:%u Socket:%lld OP:%d\n", _DEBUG_TRACE, m_ClientSockid, m_OpType);
#endif
}

/*
 * ServerConfig
 */
network::ServerConfig::ServerConfig(unsigned int _Port/* = 0*/, unsigned int _MaxConnect/* = SOMAXCONN*/) :
    M_Port(_Port),
    O_MaxConnect(_MaxConnect),
    O_MaxPostAccept(DEFAULT_MAX_POST_ACCEPT),
    O_MaxBufferLen(DEFAULT_MAX_BUFFER_LEN),
    O0_WorkerThreadsPerProcessor(DEFAULT_WORKER_THREADS_PER_PROCESSOR),
    O0_WorkerThreads(0) {
}

/*
 * Server
 */
network::Server::Server() {
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

bool network::Server::Send(SOCKET _Sockid, const char * _SendBuffer, size_t _BufferLen) {
    SVR_SOCKET_CONTEXT *sock_ctx = new SVR_SOCKET_CONTEXT(_Sockid, _SendBuffer, _BufferLen);

    return _PostSend(sock_ctx);
}

bool network::Server::CloseClient(SOCKET _Sockid) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("Close Socket:%lld @Close\n", _Sockid);
#endif

    closesocket(_Sockid);

    return true;
}

bool network::Server::Stop() {
    //TODO errcheck
    closesocket(m_Sockid);

    CloseHandle(m_CompletionPort);

    return true;
}

void network::Server::OnAccepted(SVR_SOCKET_CONTEXT * sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("OnAccepted DEBUG_TRACE %u BytesTransferred:%u @OnAccepted\n", sock_ctx->_DEBUG_TRACE, sock_ctx->m_BytesTransferred);
#endif

    if (sock_ctx->m_BytesTransferred) {
        sock_ctx->m_szBuffer[sock_ctx->m_BytesTransferred] = '\0';
        printf("%s\n", sock_ctx->m_szBuffer);
    }
}

void network::Server::OnRecvd(SVR_SOCKET_CONTEXT * sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("OnRecvd DEBUG_TRACE %u BytesTransferred:%u @OnRecvd\n", sock_ctx->_DEBUG_TRACE, sock_ctx->m_BytesTransferred);
#endif
    sock_ctx->m_szBuffer[sock_ctx->m_BytesTransferred] = '\0';
    printf("%s\n", sock_ctx->m_szBuffer);
}

void network::Server::OnSent(SVR_SOCKET_CONTEXT * sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("OnSent DEBUG_TRACE %u BytesTransferred:%u @OnSent\n", sock_ctx->_DEBUG_TRACE, sock_ctx->m_BytesTransferred);
#endif
}

void network::Server::OnClosed(SVR_SOCKET_CONTEXT * sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("OnClosed DEBUG_TRACE %u BytesTransferred:%u @OnClosed\n", sock_ctx->_DEBUG_TRACE, sock_ctx->m_BytesTransferred);
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

bool network::Server::_InitSock(unsigned int _Port, unsigned int _MaxConnect) {
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
    m_Sockid = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (m_Sockid == INVALID_SOCKET) {
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
    if (bind(m_Sockid, (SOCKADDR*)&_Addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Bind Socket @_InitSock\n");
#endif
        return false;
    }

    //LISTEN
    if (listen(m_Sockid, _MaxConnect) == SOCKET_ERROR) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Listen Port %d @_InitSock\n", _Port);
#endif
        return false;
    }

    //COMPLETIONPORT
    if (CreateIoCompletionPort((HANDLE)m_Sockid, m_CompletionPort, (ULONG_PTR)NULL, 0) == NULL) {
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
        m_Sockid,
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
        m_Sockid,
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
    for (unsigned int i = 0; i < m_ServerConfig.O_MaxPostAccept; ++i) {
        SVR_SOCKET_CONTEXT *sock_ctx = new SVR_SOCKET_CONTEXT();
        if (_PostAccept(sock_ctx) == false) {
#if(DEBUG&DEBUG_LOG)
            LOG(CC_RED, "Faild to Post Accept%d @_InitSock\n", i);
#endif
        }
    }

    return true;
}

bool network::Server::_Start(unsigned int _Port, unsigned int _MaxConnect) {
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

bool network::Server::_PostAccept(SVR_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("PostAccept DEBUG_TRACE %u\n", sock_ctx->_DEBUG_TRACE);
#endif

    DWORD _Flags = 0;

    sock_ctx->m_OpType = SVR_OP::SVROP_ACCEPTING;
    sock_ctx->m_ClientSockid = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);

    if (m_pAcceptEx(m_Sockid,
        sock_ctx->m_ClientSockid,
        sock_ctx->m_wsaBuf.buf,
#if(FEATURE_RECV_ON_ACCEPT)
        sock_ctx->m_wsaBuf.len - ((sizeof(SOCKADDR_IN) + 16) * 2),
#else
        0,
#endif
        sizeof(SOCKADDR_IN) + 16,
        sizeof(SOCKADDR_IN) + 16,
        &_Flags,
        &(sock_ctx->m_Overlapped)) == false) {
        if (WSAGetLastError() != WSA_IO_PENDING) {
#if(DEBUG&DEBUG_LOG)
            LOG(CC_RED, "Faild to Post Accept Socket:%lld @PostAccept\n", sock_ctx->m_ClientSockid);
#endif
            return false;
        }
    }

    return true;
}

bool network::Server::_PostRecv(SVR_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("PostRecv DEBUG_TRACE %u\n", sock_ctx->_DEBUG_TRACE);
#endif

    DWORD dwFlags = 0;
    DWORD dwBytes = 0;

    sock_ctx->m_OpType = SVR_OP::SVROP_RECVING;

    if (WSARecv(sock_ctx->m_ClientSockid, &(sock_ctx->m_wsaBuf), 1, &dwBytes, &dwFlags, &(sock_ctx->m_Overlapped), NULL) == SOCKET_ERROR &&
        WSAGetLastError() != WSA_IO_PENDING) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Post Recv Socket:%lld @_PostRecv\n", sock_ctx->m_ClientSockid);
#endif
        return false;
    }

    return true;
}

bool network::Server::_PostSend(SVR_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("PostSend DEBUG_TRACE %u\n", sock_ctx->_DEBUG_TRACE);
#endif

    if (sock_ctx->m_wsaBuf.len == 0) {
        return true;
    }

    DWORD dwFlags = 0;
    DWORD dwBytes = 0;

    sock_ctx->m_OpType = SVR_OP::SVROP_SENDING;

    if (WSASend(sock_ctx->m_ClientSockid, &(sock_ctx->m_wsaBuf), 1, &dwBytes, dwFlags, &(sock_ctx->m_Overlapped), NULL) == SOCKET_ERROR &&
        WSAGetLastError() != WSA_IO_PENDING) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Post Send Socket:%lld @_PostSend\n", sock_ctx->m_ClientSockid);
#endif
        return false;
    }

    return true;
}

bool network::Server::_DoAccepted(SVR_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("DoAccept DEBUG_TRACE %u Socket:%d @_DoAccepted\n", sock_ctx->_DEBUG_TRACE, sock_ctx->m_ClientSockid);
#endif

#if(DEBUG&DEBUG_LOG)
    LOG(CC_YELLOW, "Accepted a Client Socket:%lld @_DoAccepted\n", sock_ctx->m_ClientSockid);
#endif

    OnAccepted(sock_ctx);

    sock_ctx->RESET_BUFFER();

    SVR_SOCKET_CONTEXT *_NewSocketContex = new SVR_SOCKET_CONTEXT();
    _NewSocketContex->m_OpType = SVR_OP::SVROP_RECVING;
    _NewSocketContex->m_ClientSockid = sock_ctx->m_ClientSockid;
    _NewSocketContex->m_Extra = sock_ctx->m_Extra;
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

    if (CreateIoCompletionPort((HANDLE)_NewSocketContex->m_ClientSockid, m_CompletionPort, (ULONG_PTR)_NewSocketContex, 0) == NULL) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Bind Socket with CompletionPort Socket:%lld @_DoAccepted\n", sock_ctx->m_ClientSockid);
#endif
        return false;
    }

    if (_PostRecv(_NewSocketContex) == false) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Post Recv Socket:%lld @_DoAccepted\n", sock_ctx->m_ClientSockid);
#endif
        return false;
    }

    if (_PostAccept(sock_ctx) == false) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Post Accept Socket:%lld @_DoAccepted\n", sock_ctx->m_ClientSockid);
#endif
        return false;
    }

    return true;
}

bool network::Server::_DoRecvd(SVR_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("DoRecv DEBUG_TRACE %u @_DoRecvd\n", sock_ctx->_DEBUG_TRACE);
#endif

    OnRecvd(sock_ctx);

    sock_ctx->RESET_BUFFER();

    sock_ctx->m_OpType = SVR_OP::SVROP_RECVING;

    if (_PostRecv(sock_ctx) == false) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Post Recv Socket:%lld @_DoRecvd\n", sock_ctx->m_ClientSockid);
#endif
        return false;
    }

    return true;
}

bool network::Server::_DoSent(SVR_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("DoSend DEBUG_TRACE %u @_DoSent\n", sock_ctx->_DEBUG_TRACE);
#endif

    OnSent(sock_ctx);

    delete sock_ctx;//TODO use repeatedly

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
    SVR_SOCKET_CONTEXT *sock_ctx;
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

        _Ret = GetQueuedCompletionStatus(_CompletionPort, &_BytesTransferred, (PULONG_PTR)&sock_ctx, (LPOVERLAPPED*)&_Overlapped, INFINITE);

        if (_Ret == false/* && _Overlapped == NULL*/) {
#if(DEBUG&DEBUG_LOG)
            LOG(CC_RED, "GetQueuedCompletionStatus Failed @ServerWorkThread\n");
#endif

            _ErrCode = GetLastError();

            if (_ErrCode == WAIT_TIMEOUT) {
                if (!_IsClientAlive(sock_ctx->m_ClientSockid)) {
#if(DEBUG&DEBUG_LOG)
                    LOG(CC_YELLOW, "Client Offline Error Code:%ld Socket:%lld @ServerWorkThread\n", WAIT_TIMEOUT, sock_ctx->m_ClientSockid);
#endif
                    _OnClosed(_Server, sock_ctx);

                    closesocket(sock_ctx->m_ClientSockid);

                    delete sock_ctx;
                }
                continue;
            } else if (_ErrCode == ERROR_NETNAME_DELETED) {
                if (sock_ctx) {
#if(DEBUG&DEBUG_LOG)
                    LOG(CC_YELLOW, "Client Offline Error Code:%ld Socket:%lld @ServerWorkThread\n", ERROR_NETNAME_DELETED, sock_ctx->m_ClientSockid);
#endif
                    _OnClosed(_Server, sock_ctx);

                    closesocket(sock_ctx->m_ClientSockid);

                    delete sock_ctx;
                } else {
                    // TODO
                }
            } else if (_ErrCode == ERROR_ABANDONED_WAIT_0) {
                // TODO
                continue;
            } else {
                // TODO
                continue;
            }

            continue;
        }

        sock_ctx = CONTAINING_RECORD(_Overlapped, SVR_SOCKET_CONTEXT, m_Overlapped);

        if (_BytesTransferred == 0 && sock_ctx->m_OpType != SVR_OP::SVROP_ACCEPTING) {
#if(DEBUG&DEBUG_LOG)
            LOG(CC_YELLOW, "Client Offline Zero Bytes Transferred Socket:%lld @ServerWorkThread\n", sock_ctx->m_ClientSockid);
#endif
            _OnClosed(_Server, sock_ctx);

            closesocket(sock_ctx->m_ClientSockid);

            delete sock_ctx;
        }

        sock_ctx->m_BytesTransferred = _BytesTransferred;

        switch (sock_ctx->m_OpType) {
        case SVR_OP::SVROP_ACCEPTING:
            _DoAccepted(_Server, sock_ctx);
            break;
        case SVR_OP::SVROP_RECVING:
            _DoRecvd(_Server, sock_ctx);
            break;
        case SVR_OP::SVROP_SENDING:
            _DoSent(_Server, sock_ctx);
            break;
        default:
            break;
        }
    }

    return 0;
}

/*
 * CLT_SOCKET_CONTEXT
 */
network::CLT_SOCKET_CONTEXT::CLT_SOCKET_CONTEXT(size_t _MaxBufferLen /*= DEFAULT_MAX_BUFFER_LEN*/) :
    m_OpType(CLT_OP::CLTOP_NULL),
    m_Extra(NULL) {
    m_szBuffer = new char[_MaxBufferLen];
    m_wsaBuf.buf = m_szBuffer;
    m_wsaBuf.len = (ULONG)_MaxBufferLen;

    memset(&m_Overlapped, 0, sizeof(OVERLAPPED));

#if(DEBUG&DEBUG_TRACE)
    _DEBUG_TRACE = network::_DEBUG_TRACE++;
#endif
}

network::CLT_SOCKET_CONTEXT::CLT_SOCKET_CONTEXT(char *_Buffer, size_t _BufferLen) :
    m_OpType(CLT_OP::CLTOP_NULL),
    m_Extra(NULL) {
    m_szBuffer = NULL;
    m_wsaBuf.buf = _Buffer;
    m_wsaBuf.len = (ULONG)_BufferLen;

    memset(&m_Overlapped, 0, sizeof(OVERLAPPED));

#if(DEBUG&DEBUG_TRACE)
    _DEBUG_TRACE = network::_DEBUG_TRACE++;
#endif
}

network::CLT_SOCKET_CONTEXT::CLT_SOCKET_CONTEXT(const char *_Buffer, size_t _BufferLen) :
    m_OpType(CLT_OP::CLTOP_NULL),
    m_Extra(NULL) {
    m_szBuffer = new char[_BufferLen];
    memcpy(m_szBuffer, _Buffer, sizeof(char)*_BufferLen);
    m_wsaBuf.buf = m_szBuffer;
    m_wsaBuf.len = (ULONG)_BufferLen;

    memset(&m_Overlapped, 0, sizeof(OVERLAPPED));

#if(DEBUG&DEBUG_TRACE)
    _DEBUG_TRACE = network::_DEBUG_TRACE++;
#endif
}

void network::CLT_SOCKET_CONTEXT::RESET_BUFFER() {
    m_BytesTransferred = 0;
}

network::CLT_SOCKET_CONTEXT::~CLT_SOCKET_CONTEXT() {
    if (m_szBuffer)
        delete[] m_szBuffer;

    static int count = 0;
    printf("socket context disposed,%d\n", count++);
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("PSC Dispose DEBUG_TRACE:%d\n", _DEBUG_TRACE);
#endif
}

/*
 * ClientConfig
 */
network::ClientConfig::ClientConfig() :
    O0_WorkerThreadsPerProcessor(0),
    O0_WorkerThreads(2) {
}

/*
 * Client
 */
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

SOCKET network::Client::Connect(const char * _Address, unsigned int _Port, unsigned int * _LocalPort/* = NULL*/) {
    unsigned int _LocalPort1 = 0;
    if (!_LocalPort) {
        _LocalPort = &_LocalPort1;
    }

    m_Sockid = _InitSock(_LocalPort);
    if (*_LocalPort < 0) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Init Sock @Connect\n");
#endif
        return -2;
    }

    if (_PostConnect(m_Sockid, inet_addr(_Address), _Port) == false) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Post Connect @Connect\n");
#endif
        return -3;
    }

    return m_Sockid;
}

bool network::Client::Send(char *_SendBuffer, size_t _BufferLen) {
    CLT_SOCKET_CONTEXT *sock_ctx = new CLT_SOCKET_CONTEXT(_SendBuffer, _BufferLen);

    return _PostSend(sock_ctx);
}

bool network::Client::Send(const char *_SendBuffer, size_t _BufferLen) {
    CLT_SOCKET_CONTEXT *sock_ctx = new CLT_SOCKET_CONTEXT(_SendBuffer, _BufferLen);

    return _PostSend(sock_ctx);
}

bool network::Client::Close() {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("Close Socket:%lld @Close\n", m_Sockid);
#endif

    //shutdown(m_Sockid, SD_BOTH);

    //LINGER _Linger = { 1,0 };
    //_Linger.l_onoff = 0;
    //setsockopt(m_Sockid, SOL_SOCKET, SO_LINGER, (const char *)&_Linger, sizeof(_Linger));

    closesocket(m_Sockid);

    CloseHandle(m_CompletionPort);

    return true;
}

void network::Client::OnConnected(CLT_SOCKET_CONTEXT * sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("OnConnected DEBUG_TRACE %u BytesTransferred:%u @OnConnected\n", sock_ctx->_DEBUG_TRACE, sock_ctx->m_BytesTransferred);
#endif

    if (sock_ctx->m_BytesTransferred) {
        sock_ctx->m_szBuffer[sock_ctx->m_BytesTransferred] = '\0';
        printf("%s\n", sock_ctx->m_szBuffer);
    }
}

void network::Client::OnSent(CLT_SOCKET_CONTEXT * sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("OnSent DEBUG_TRACE %u BytesTransferred:%u @OnSent\n", sock_ctx->_DEBUG_TRACE, sock_ctx->m_BytesTransferred);
#endif
}

void network::Client::OnRecvd(CLT_SOCKET_CONTEXT * sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("OnRecvd DEBUG_TRACE %u BytesTransferred:%u @OnRecvd\n", sock_ctx->_DEBUG_TRACE, sock_ctx->m_BytesTransferred);
#endif

    sock_ctx->m_szBuffer[sock_ctx->m_BytesTransferred] = '\0';
    printf("%s\n", sock_ctx->m_szBuffer);
}

void network::Client::OnClosed(CLT_SOCKET_CONTEXT * sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("OnClosed DEBUG_TRACE %u BytesTransferred:%u @OnClosed\n", sock_ctx->_DEBUG_TRACE, sock_ctx->m_BytesTransferred);
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

SOCKET network::Client::_InitSock(unsigned int *_Port) {
    //SOCKET
    SOCKET _Sockid = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (_Sockid == INVALID_SOCKET) {
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
    if (bind(_Sockid, (SOCKADDR*)&_LocalAddr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Bind Socket @_InitSock\n");
#endif
        return -4;
    }

    if (CreateIoCompletionPort((HANDLE)_Sockid, m_CompletionPort, (ULONG_PTR)NULL, 0) == NULL) {
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
            _Sockid,
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

    return _Sockid;
}

bool network::Client::_PostConnect(SOCKET _Sockid, unsigned long _Ip, unsigned int _Port) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("PostConnect @_PostConnect\n");
#endif

    SOCKADDR_IN _Addr;
    memset(&_Addr, 0, sizeof(SOCKADDR_IN));
    _Addr.sin_family = AF_INET;
    _Addr.sin_addr.S_un.S_addr = _Ip;
    _Addr.sin_port = htons(_Port);

    CLT_SOCKET_CONTEXT *sock_ctx = new CLT_SOCKET_CONTEXT();

    sock_ctx->m_OpType = CLT_OP::CLTOP_CONNECTING;

    DWORD dwBytes = 0;
    if (m_ConnectEx(_Sockid, (SOCKADDR*)&_Addr, sizeof(_Addr), NULL, 0, &dwBytes, (LPOVERLAPPED)sock_ctx) == false) {
        if (WSAGetLastError() != ERROR_IO_PENDING) {
#if(DEBUG&DEBUG_LOG)
            LOG(CC_RED, "Faild to Post Connect%d @_PostConnect\n");
#endif
            return false;
        }
    }

    return true;
}

bool network::Client::_PostSend(CLT_SOCKET_CONTEXT * sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("PostSend DEBUG_TRACE %u @_PostSend\n", sock_ctx->_DEBUG_TRACE);
#endif

    /*if (sock_ctx->m_wsaBuf.len == 0) {
        return true;
    }*/

    DWORD dwFlags = 0;
    DWORD dwBytes = 0;

    sock_ctx->m_OpType = CLT_OP::CLTOP_SENDING;

    if (WSASend(m_Sockid, &(sock_ctx->m_wsaBuf), 1, &dwBytes, dwFlags, &(sock_ctx->m_Overlapped), NULL) == SOCKET_ERROR) {
        if (WSAGetLastError() != WSA_IO_PENDING) {
#if(DEBUG&DEBUG_LOG)
            LOG(CC_RED, "Faild to Post Send %d @_PostSend\n", WSAGetLastError());
#endif
            return false;
        }
    }

    return true;
}

bool network::Client::_PostRecv(CLT_SOCKET_CONTEXT * sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("PostRecv DEBUG_TRACE %u @_PostRecv\n", sock_ctx->_DEBUG_TRACE);
#endif

    DWORD dwFlags = 0;
    DWORD dwBytes = 0;

    sock_ctx->m_OpType = CLT_OP::CLTOP_RECVING;

    if (WSARecv(m_Sockid, &(sock_ctx->m_wsaBuf), 1, &dwBytes, &dwFlags, &(sock_ctx->m_Overlapped), NULL) == SOCKET_ERROR) {
        if (WSAGetLastError() != WSA_IO_PENDING) {
#if(DEBUG&DEBUG_LOG)
            LOG(CC_RED, "Faild to Post Recv @_PostRecv\n");
#endif
            return false;
        }
    }

    return true;
}

bool network::Client::_DoConnected(CLT_SOCKET_CONTEXT * sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("DoConnected DEBUG_TRACE %u @_DoConnected\n", sock_ctx->_DEBUG_TRACE);
#endif

    OnConnected(sock_ctx);

    sock_ctx->RESET_BUFFER();

    if (_PostRecv(sock_ctx) == false) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Post Recv @_DoConnected\n");
#endif
        return false;
    }

    return true;
}

bool network::Client::_DoSent(CLT_SOCKET_CONTEXT * sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("DoSent DEBUG_TRACE %u @_DoSent\n", sock_ctx->_DEBUG_TRACE);
#endif

    OnSent(sock_ctx);

    delete sock_ctx;//TODO use repeatedly

    return false;
}

bool network::Client::_DoRecvd(CLT_SOCKET_CONTEXT * sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("DoRecvd DEBUG_TRACE %u @_DoRecvd\n", sock_ctx->_DEBUG_TRACE);
#endif

    OnRecvd(sock_ctx);

    _PostRecv(sock_ctx);

    return false;
}

DWORD network::Client::ClientWorkThread(LPVOID _LpParam) {
    WORKER_PARAMS<Client> *_WorkerParams = (WORKER_PARAMS<Client>*)_LpParam;

#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("ClientWorkThread ThreadNo:%u @ClientWorkThread\n", _WorkerParams->m_ThreadNo);
#endif

    DWORD _BytesTransferred;
    OVERLAPPED *_Overlapped;
    CLT_SOCKET_CONTEXT *sock_ctx;
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

        _Ret = GetQueuedCompletionStatus(_CompletionPort, &_BytesTransferred, (PULONG_PTR)&sock_ctx, (LPOVERLAPPED*)&_Overlapped, INFINITE);

        if (_Ret == false) {
#if(DEBUG&DEBUG_LOG)
            LOG(CC_RED, "GetQueuedCompletionStatus Failed @ClientWorkThread\n");
#endif

            _ErrCode = GetLastError();

            if (_ErrCode == WAIT_TIMEOUT) {
                if (!_IsServerAlive(_Client->m_Sockid)) {
#if(DEBUG&DEBUG_LOG)
                    LOG(CC_YELLOW, "Server Error Socket:%lld @ClientWorkThread\n", _Client->m_Sockid);
#endif
                    _OnClosed(_Client, sock_ctx);

                    closesocket(_Client->m_Sockid);

                    delete sock_ctx;
                }
                continue;
            } else if (_ErrCode == ERROR_NETNAME_DELETED) {
                if (sock_ctx) {
#if(DEBUG&DEBUG_LOG)
                    LOG(CC_YELLOW, "Server Error Socket:%lld @ClientWorkThread\n", _Client->m_Sockid);
#endif
                    _OnClosed(_Client, sock_ctx);

                    closesocket(_Client->m_Sockid);

                    delete sock_ctx;
                }

                continue;
            } else if (_ErrCode == ERROR_ABANDONED_WAIT_0) {
                // TODO
                continue;
            } else {
                // TODO
                continue;
            }
        }

        sock_ctx = CONTAINING_RECORD(_Overlapped, CLT_SOCKET_CONTEXT, m_Overlapped);

        if (_BytesTransferred == 0 && sock_ctx->m_OpType != CLT_OP::CLTOP_CONNECTING) {
#if(DEBUG&DEBUG_LOG)
            LOG(CC_YELLOW, "Server Offline Socket:%lld @ClientWorkThread\n", _Client->m_Sockid);
#endif
            _OnClosed(_Client, sock_ctx);

            closesocket(_Client->m_Sockid);

            delete sock_ctx;

            continue;
        }

        sock_ctx->m_BytesTransferred = _BytesTransferred;

        switch (sock_ctx->m_OpType) {
        case CLT_OP::CLTOP_CONNECTING:
            _DoConnected(_Client, sock_ctx);
            break;
        case CLT_OP::CLTOP_SENDING:
            _DoSent(_Client, sock_ctx);
            break;
        case CLT_OP::CLTOP_RECVING:
            _DoRecvd(_Client, sock_ctx);
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