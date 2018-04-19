
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
network::SVR_SOCKET_CONTEXT::SVR_SOCKET_CONTEXT(size_t max_buffer_len/* = DEFAULT_MAX_BUFFER_LEN*/) :
    m_bytes_transferred(0),
    m_op_type(SVR_OP::SVROP_NULL),
    m_client_sockid(INVALID_SOCKET),
    m_extra(NULL) {
    m_buffer = new char[max_buffer_len];//TODO user-defined(upper layer) buffer len
    m_wsa_buf.buf = m_buffer;
    m_wsa_buf.len = (ULONG)max_buffer_len;

    memset(&m_OVERLAPPED, 0, sizeof(OVERLAPPED));

#if(DEBUG&DEBUG_TRACE)
    _DEBUG_TRACE = network::_DEBUG_TRACE++;
#endif
}

network::SVR_SOCKET_CONTEXT::SVR_SOCKET_CONTEXT(SOCKET sockid, char *buffer, size_t buffer_len) :
    m_bytes_transferred(0),
    m_op_type(SVR_OP::SVROP_NULL),
    m_client_sockid(sockid),
    m_extra(NULL) {
    m_buffer = buffer;
    m_wsa_buf.buf = m_buffer;
    m_wsa_buf.len = (ULONG)buffer_len;

    memset(&m_OVERLAPPED, 0, sizeof(OVERLAPPED));

#if(DEBUG&DEBUG_TRACE)
    _DEBUG_TRACE = network::_DEBUG_TRACE++;
#endif
}

network::SVR_SOCKET_CONTEXT::SVR_SOCKET_CONTEXT(SOCKET sockid, const char *buffer, size_t buffer_len) :
    m_bytes_transferred(0),
    m_op_type(SVR_OP::SVROP_NULL),
    m_client_sockid(sockid),
    m_extra(NULL) {
    m_buffer = new char[buffer_len];
    memcpy(m_buffer, buffer, sizeof(char)*buffer_len);
    m_wsa_buf.buf = m_buffer;
    m_wsa_buf.len = (ULONG)buffer_len;

    memset(&m_OVERLAPPED, 0, sizeof(OVERLAPPED));

#if(DEBUG&DEBUG_TRACE)
    _DEBUG_TRACE = network::_DEBUG_TRACE++;
#endif
}

void network::SVR_SOCKET_CONTEXT::RESET_BUFFER() {
    m_bytes_transferred = 0;
}

network::SVR_SOCKET_CONTEXT::~SVR_SOCKET_CONTEXT() {
    if (m_buffer)
        delete[] m_buffer;

#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("PSC Dispose DEBUG_TRACE:%u Socket:%lld OP:%d\n", _DEBUG_TRACE, m_client_sockid, m_op_type);
#endif
}

/*
 * ServerConfig
 */
network::ServerConfig::ServerConfig(unsigned int max_connect/* = SOMAXCONN*/) :
    o_max_connect(max_connect),
    o_max_post_accept(DEFAULT_MAX_POST_ACCEPT),
    o_max_buffer_len(DEFAULT_MAX_BUFFER_LEN),
    o0_worker_threads_per_processor(DEFAULT_WORKER_THREADS_PER_PROCESSOR),
    o0_worker_threads(0) {
}

/*
 * Server
 */
network::Server::Server() {
}

network::Server::Server(const ServerConfig &serverconfig) {
    m_server_config = serverconfig;
}

void network::Server::set_config(const ServerConfig &serverconfig) {
    m_server_config = serverconfig;
}

bool network::Server::start_listen(unsigned int port) {
    return _start(port, m_server_config.o_max_connect);
}

bool network::Server::send(SOCKET sockid, const char *buffer, size_t buffer_len) {
    SVR_SOCKET_CONTEXT *sock_ctx = new SVR_SOCKET_CONTEXT(sockid, buffer, buffer_len);

    return _post_send(sock_ctx);
}

bool network::Server::close_client(SOCKET sockid) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("Close Socket:%lld @Close\n", sockid);
#endif

    closesocket(sockid);

    return true;
}

bool network::Server::close() {
    //TODO errcheck
    if (m_sockid != SOCKET_ERROR) {
        closesocket(m_sockid);
    }
    if (m_completion_port) {
        CloseHandle(m_completion_port);
        m_completion_port = NULL;
    }
    return true;
}

void network::Server::on_accepted(SVR_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("OnAccepted DEBUG_TRACE %u BytesTransferred:%u @OnAccepted\n", sock_ctx->_DEBUG_TRACE, sock_ctx->m_bytes_transferred);
#endif

    if (sock_ctx->m_bytes_transferred) {
        sock_ctx->m_buffer[sock_ctx->m_bytes_transferred] = '\0';
        printf("%s\n", sock_ctx->m_buffer);
    }
}

void network::Server::on_recvd(SVR_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("OnRecvd DEBUG_TRACE %u BytesTransferred:%u @OnRecvd\n", sock_ctx->_DEBUG_TRACE, sock_ctx->m_bytes_transferred);
#endif
    sock_ctx->m_buffer[sock_ctx->m_bytes_transferred] = '\0';
    printf("%s\n", sock_ctx->m_buffer);
}

void network::Server::on_sent(SVR_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("OnSent DEBUG_TRACE %u BytesTransferred:%u @OnSent\n", sock_ctx->_DEBUG_TRACE, sock_ctx->m_bytes_transferred);
#endif
}

void network::Server::on_closed(SVR_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("OnClosed DEBUG_TRACE %u BytesTransferred:%u @OnClosed\n", sock_ctx->_DEBUG_TRACE, sock_ctx->m_bytes_transferred);
#endif
}

bool network::Server::_init_complition_port() {
    //Completion Port
    m_completion_port = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (m_completion_port == NULL) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Create ComplitionPort @_InitComplitionPort\n");
#endif
        return false;
    }

    unsigned int worker_threads_num = m_server_config.o0_worker_threads > 0 ? m_server_config.o0_worker_threads : (_GetProcessorNum()*m_server_config.o0_worker_threads_per_processor);
    if (worker_threads_num <= 0) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Invalid Worker Threads Number @_InitComplitionPort\n");
#endif
        return false;
    }

    HANDLE* worker_threads = new HANDLE[worker_threads_num];
    memset(worker_threads, 0, sizeof(HANDLE)*worker_threads_num);
    DWORD ThreadId;
    for (unsigned int i = 0; i < worker_threads_num; ++i) {
        WORKER_PARAMS<Server> *_pWorkerParams = new WORKER_PARAMS<Server>();
        _pWorkerParams->m_instance = this;
        _pWorkerParams->m_thread_num = i;

        worker_threads[i] = CreateThread(NULL, 0, ServerWorkThread, _pWorkerParams, 0, &ThreadId);
        if (worker_threads[i] == NULL) {
#if(DEBUG&DEBUG_LOG)
            LOG(CC_RED, "Failed to Start Thread%u @_InitComplitionPort\n", i);
#endif
        }
    }

    return true;
}

bool network::Server::_init_sock(unsigned int port, unsigned int max_connect) {
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
    m_sockid = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (m_sockid == INVALID_SOCKET) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Create Socket @_InitSock\n");
#endif
        return false;
    }

    SOCKADDR_IN addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    //BIND
    if (bind(m_sockid, (SOCKADDR*)&addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Bind Socket @_InitSock\n");
#endif
        return false;
    }

    //LISTEN
    if (listen(m_sockid, max_connect) == SOCKET_ERROR) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Listen Port %d @_InitSock\n", port);
#endif
        return false;
    }

    //COMPLETIONPORT
    if (CreateIoCompletionPort((HANDLE)m_sockid, m_completion_port, (ULONG_PTR)NULL, 0) == NULL) {
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
        m_sockid,
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
        m_sockid,
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
    for (unsigned int i = 0; i < m_server_config.o_max_post_accept; ++i) {
        SVR_SOCKET_CONTEXT *sock_ctx = new SVR_SOCKET_CONTEXT();
        if (_post_accept(sock_ctx) == false) {
#if(DEBUG&DEBUG_LOG)
            LOG(CC_RED, "Faild to Post Accept%d @_InitSock\n", i);
#endif
        }
    }

    return true;
}

bool network::Server::_start(unsigned int port, unsigned int max_connect) {
    if (_init_complition_port() == false) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Init ComplitionPort @_Start\n");
#endif
        return false;
    }

    if (_init_sock(port, max_connect) == false) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Init Sock @_Start\n");
#endif
        return false;
    }

    return true;
}

bool network::Server::_post_accept(SVR_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("PostAccept DEBUG_TRACE %u\n", sock_ctx->_DEBUG_TRACE);
#endif

    DWORD flags = 0;

    sock_ctx->m_op_type = SVR_OP::SVROP_ACCEPTING;
    sock_ctx->m_client_sockid = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);

    if (m_pAcceptEx(m_sockid,
        sock_ctx->m_client_sockid,
        sock_ctx->m_wsa_buf.buf,
#if(FEATURE_RECV_ON_ACCEPT)
        sock_ctx->m_wsaBuf.len - ((sizeof(SOCKADDR_IN) + 16) * 2),
#else
        0,
#endif
        sizeof(SOCKADDR_IN) + 16,
        sizeof(SOCKADDR_IN) + 16,
        &flags,
        &(sock_ctx->m_OVERLAPPED)) == false) {
        if (WSAGetLastError() != WSA_IO_PENDING) {
#if(DEBUG&DEBUG_LOG)
            LOG(CC_RED, "Faild to Post Accept Socket:%lld @PostAccept\n", sock_ctx->m_client_sockid);
#endif
            return false;
        }
    }

    return true;
}

bool network::Server::_post_recv(SVR_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("PostRecv DEBUG_TRACE %u\n", sock_ctx->_DEBUG_TRACE);
#endif

    DWORD bytes = 0;
    DWORD flags = 0;

    sock_ctx->m_op_type = SVR_OP::SVROP_RECVING;

    if (WSARecv(sock_ctx->m_client_sockid, &(sock_ctx->m_wsa_buf), 1, &bytes, &flags, &(sock_ctx->m_OVERLAPPED), NULL) == SOCKET_ERROR &&
        WSAGetLastError() != WSA_IO_PENDING) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Post Recv Socket:%lld @_PostRecv\n", sock_ctx->m_client_sockid);
#endif
        return false;
    }

    return true;
}

bool network::Server::_post_send(SVR_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("PostSend DEBUG_TRACE %u\n", sock_ctx->_DEBUG_TRACE);
#endif

    if (sock_ctx->m_wsa_buf.len == 0) {
        return true;
    }

    DWORD bytes = 0;
    DWORD flags = 0;

    sock_ctx->m_op_type = SVR_OP::SVROP_SENDING;

    if (WSASend(sock_ctx->m_client_sockid, &(sock_ctx->m_wsa_buf), 1, &bytes, flags, &(sock_ctx->m_OVERLAPPED), NULL) == SOCKET_ERROR &&
        WSAGetLastError() != WSA_IO_PENDING) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Post Send Socket:%lld @_PostSend\n", sock_ctx->m_client_sockid);
#endif
        return false;
    }

    return true;
}

bool network::Server::_do_accepted(SVR_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("DoAccept DEBUG_TRACE %u Socket:%d @_DoAccepted\n", sock_ctx->_DEBUG_TRACE, sock_ctx->m_client_sockid);
#endif

#if(DEBUG&DEBUG_LOG)
    LOG(CC_YELLOW, "Accepted a Client Socket:%lld @_DoAccepted\n", sock_ctx->m_client_sockid);
#endif

    SVR_SOCKET_CONTEXT *new_sock_ctx = new SVR_SOCKET_CONTEXT();
    new_sock_ctx->m_op_type = SVR_OP::SVROP_RECVING;
    new_sock_ctx->m_client_sockid = sock_ctx->m_client_sockid;
    new_sock_ctx->m_extra = sock_ctx->m_extra;
    SOCKADDR_IN *_ClientAddr, *_LocalAddr;//mark:need to delete?
    int _ClientAddrLen = sizeof(SOCKADDR_IN), _LocalAddrLen = sizeof(SOCKADDR_IN);

    m_pGetAcceptExSockAddrs(
        new_sock_ctx->m_wsa_buf.buf,
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

    new_sock_ctx->m_client_addr = *_ClientAddr;//mark

    //TODO save clientaddr

    if (CreateIoCompletionPort((HANDLE)new_sock_ctx->m_client_sockid, m_completion_port, (ULONG_PTR)new_sock_ctx, 0) == NULL) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Bind Socket with CompletionPort Socket:%lld @_DoAccepted\n", sock_ctx->m_client_sockid);
#endif
        return false;
    }

    if (_post_recv(new_sock_ctx) == false) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Post Recv Socket:%lld @_DoAccepted\n", sock_ctx->m_client_sockid);
#endif
        return false;
    }

    if (_post_accept(sock_ctx) == false) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Post Accept Socket:%lld @_DoAccepted\n", sock_ctx->m_client_sockid);
#endif
        return false;
    }

    on_accepted(sock_ctx);

    //sock_ctx->RESET_BUFFER();

    return true;
}

bool network::Server::_do_recvd(SVR_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("DoRecv DEBUG_TRACE %u @_DoRecvd\n", sock_ctx->_DEBUG_TRACE);
#endif

    on_recvd(sock_ctx);

    sock_ctx->RESET_BUFFER();

    sock_ctx->m_op_type = SVR_OP::SVROP_RECVING;

    if (_post_recv(sock_ctx) == false) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Post Recv Socket:%lld @_DoRecvd\n", sock_ctx->m_client_sockid);
#endif
        return false;
    }

    return true;
}

bool network::Server::_do_sent(SVR_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("DoSend DEBUG_TRACE %u @_DoSent\n", sock_ctx->_DEBUG_TRACE);
#endif

    on_sent(sock_ctx);

    delete sock_ctx;//TODO use repeatedly

    return true;
}

bool network::Server::_is_client_alive(SOCKET sockid) {
    int bytes_sent = ::send(sockid, "", 0, 0);

    return bytes_sent != -1;
}

DWORD WINAPI network::Server::ServerWorkThread(LPVOID lpParam) {
    WORKER_PARAMS<Server> *worker_params = (WORKER_PARAMS<Server>*)lpParam;

#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("ServerWorkThread ThreadNo:%u @ServerWorkThread\n", worker_params->m_thread_num);
#endif

    DWORD bytes_transferred;
    OVERLAPPED *overlapped;
    SVR_SOCKET_CONTEXT *sock_ctx;
    HANDLE completion_port = worker_params->m_instance->m_completion_port;
    Server *server = worker_params->m_instance;

    typedef bool(*DoEvent)(Server*, SVR_SOCKET_CONTEXT*);
    typedef void(*OnEvent)(Server*, SVR_SOCKET_CONTEXT*);

    DoEvent _DoAccepted = pointer_cast<DoEvent>(&Server::_do_accepted);
    DoEvent _DoRecvd = pointer_cast<DoEvent>(&Server::_do_recvd);
    DoEvent _DoSent = pointer_cast<DoEvent>(&Server::_do_sent);

    OnEvent _OnClosed = pointer_cast<OnEvent>(&Server::on_closed);

    BOOL ret;
    DWORD err_code;

    while (true) {
        bytes_transferred = 0;

        ret = GetQueuedCompletionStatus(completion_port, &bytes_transferred, (PULONG_PTR)&sock_ctx, (LPOVERLAPPED*)&overlapped, INFINITE);

        if (ret == false/* && _Overlapped == NULL*/) {
#if(DEBUG&DEBUG_LOG)
            LOG(CC_RED, "GetQueuedCompletionStatus Failed @ServerWorkThread\n");
#endif

            err_code = GetLastError();

            if (err_code == WAIT_TIMEOUT) {
                if (!_is_client_alive(sock_ctx->m_client_sockid)) {
#if(DEBUG&DEBUG_LOG)
                    LOG(CC_YELLOW, "Client Offline Error Code:%ld Socket:%lld @ServerWorkThread\n", WAIT_TIMEOUT, sock_ctx->m_client_sockid);
#endif
                    _OnClosed(server, sock_ctx);

                    closesocket(sock_ctx->m_client_sockid);

                    delete sock_ctx;
                }
                continue;
            } else if (err_code == ERROR_NETNAME_DELETED) {
                if (sock_ctx) {
#if(DEBUG&DEBUG_LOG)
                    LOG(CC_YELLOW, "Client Offline Error Code:%ld Socket:%lld @ServerWorkThread\n", ERROR_NETNAME_DELETED, sock_ctx->m_client_sockid);
#endif
                    _OnClosed(server, sock_ctx);

                    closesocket(sock_ctx->m_client_sockid);

                    delete sock_ctx;
                } else {
                    // TODO
                }
            } else if (err_code == ERROR_ABANDONED_WAIT_0) {
                // TODO
                continue;
            } else {
                // TODO
                break;
            }

            continue;
        }

        sock_ctx = CONTAINING_RECORD(overlapped, SVR_SOCKET_CONTEXT, m_OVERLAPPED);

        if (bytes_transferred == 0 && sock_ctx->m_op_type != SVR_OP::SVROP_ACCEPTING) {
#if(DEBUG&DEBUG_LOG)
            LOG(CC_YELLOW, "Client Offline Zero Bytes Transferred Socket:%lld @ServerWorkThread\n", sock_ctx->m_client_sockid);
#endif
            _OnClosed(server, sock_ctx);

            closesocket(sock_ctx->m_client_sockid);

            delete sock_ctx;
        }

        sock_ctx->m_bytes_transferred = bytes_transferred;

        switch (sock_ctx->m_op_type) {
        case SVR_OP::SVROP_ACCEPTING:
            _DoAccepted(server, sock_ctx);
            break;
        case SVR_OP::SVROP_RECVING:
            _DoRecvd(server, sock_ctx);
            break;
        case SVR_OP::SVROP_SENDING:
            _DoSent(server, sock_ctx);
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
network::CLT_SOCKET_CONTEXT::CLT_SOCKET_CONTEXT(size_t max_buffer_len /*= DEFAULT_MAX_BUFFER_LEN*/) :
    m_op_type(CLT_OP::CLTOP_NULL),
    m_extra(NULL) {
    m_buffer = new char[max_buffer_len];
    m_wsa_buf.buf = m_buffer;
    m_wsa_buf.len = (ULONG)max_buffer_len;

    memset(&m_OVERLAPPED, 0, sizeof(OVERLAPPED));

#if(DEBUG&DEBUG_TRACE)
    _DEBUG_TRACE = network::_DEBUG_TRACE++;
#endif
}

network::CLT_SOCKET_CONTEXT::CLT_SOCKET_CONTEXT(char *buffer, size_t buffer_len) :
    m_op_type(CLT_OP::CLTOP_NULL),
    m_extra(NULL) {
    m_buffer = NULL;
    m_wsa_buf.buf = buffer;
    m_wsa_buf.len = (ULONG)buffer_len;

    memset(&m_OVERLAPPED, 0, sizeof(OVERLAPPED));

#if(DEBUG&DEBUG_TRACE)
    _DEBUG_TRACE = network::_DEBUG_TRACE++;
#endif
}

network::CLT_SOCKET_CONTEXT::CLT_SOCKET_CONTEXT(const char *buffer, size_t buffer_len) :
    m_op_type(CLT_OP::CLTOP_NULL),
    m_extra(NULL) {
    m_buffer = new char[buffer_len];
    memcpy(m_buffer, buffer, sizeof(char)*buffer_len);
    m_wsa_buf.buf = m_buffer;
    m_wsa_buf.len = (ULONG)buffer_len;

    memset(&m_OVERLAPPED, 0, sizeof(OVERLAPPED));

#if(DEBUG&DEBUG_TRACE)
    _DEBUG_TRACE = network::_DEBUG_TRACE++;
#endif
}

void network::CLT_SOCKET_CONTEXT::RESET_BUFFER() {
    m_bytes_transferred = 0;
}

network::CLT_SOCKET_CONTEXT::~CLT_SOCKET_CONTEXT() {
    if (m_buffer)
        delete[] m_buffer;

#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("PSC Dispose DEBUG_TRACE:%d\n", _DEBUG_TRACE);
#endif
}

/*
 * ClientConfig
 */
network::ClientConfig::ClientConfig() :
    o0_worker_threads_per_processor(0),
    o0_Worker_threads(2) {
}

/*
 * Client
 */
network::Client::Client() :m_completion_port(NULL) {
    _init();
}

network::Client::Client(const ClientConfig &client_config) : m_completion_port(NULL) {
    m_client_config = client_config;

    _init();
}

void network::Client::set_config(const ClientConfig &client_config) {
    m_client_config = client_config;
}

int network::Client::_init() {
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

    if (_init_completion_port() == false) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Init ComplitionPort @_Init\n");
#endif
        return -3;
    }

    return 0;
}

SOCKET network::Client::connect(const char *addr, unsigned int port, unsigned int *local_port/* = NULL*/) {
    unsigned int tmp_local_port = 0;
    if (!local_port) {
        local_port = &tmp_local_port;
    }

    m_sockid = _init_sock(local_port);
    if (*local_port < 0) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Init Sock @Connect\n");
#endif
        return -2;
    }

    if (_post_connect(m_sockid, inet_addr(addr), port) == false) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Post Connect @Connect\n");
#endif
        return -3;
    }

    return m_sockid;
}

bool network::Client::send(char *buffer, size_t buffer_len) {
    CLT_SOCKET_CONTEXT *sock_ctx = new CLT_SOCKET_CONTEXT(buffer, buffer_len);

    return _post_send(sock_ctx);
}

bool network::Client::send(const char *buffer, size_t buffer_len) {
    CLT_SOCKET_CONTEXT *sock_ctx = new CLT_SOCKET_CONTEXT(buffer, buffer_len);

    return _post_send(sock_ctx);
}

bool network::Client::close() {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("Close Socket:%lld @Close\n", m_sockid);
#endif

    //shutdown(m_sockid, SD_BOTH);

    //LINGER _Linger = { 1,0 };
    //_Linger.l_onoff = 0;
    //setsockopt(m_sockid, SOL_SOCKET, SO_LINGER, (const char *)&_Linger, sizeof(_Linger));

    if (m_sockid != SOCKET_ERROR) {
        closesocket(m_sockid);
        m_sockid = SOCKET_ERROR;
    }

    if (m_completion_port) {
        CloseHandle(m_completion_port);
        m_completion_port = NULL;
    }

    return true;
}

void network::Client::on_connected(CLT_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("OnConnected DEBUG_TRACE %u BytesTransferred:%u @OnConnected\n", sock_ctx->_DEBUG_TRACE, sock_ctx->m_bytes_transferred);
#endif

    if (sock_ctx->m_bytes_transferred) {
        sock_ctx->m_buffer[sock_ctx->m_bytes_transferred] = '\0';
        printf("%s\n", sock_ctx->m_buffer);
    }
}

void network::Client::on_sent(CLT_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("OnSent DEBUG_TRACE %u BytesTransferred:%u @OnSent\n", sock_ctx->_DEBUG_TRACE, sock_ctx->m_bytes_transferred);
#endif
}

void network::Client::on_recvd(CLT_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("OnRecvd DEBUG_TRACE %u BytesTransferred:%u @OnRecvd\n", sock_ctx->_DEBUG_TRACE, sock_ctx->m_bytes_transferred);
#endif

    sock_ctx->m_buffer[sock_ctx->m_bytes_transferred] = '\0';
    printf("%s\n", sock_ctx->m_buffer);
}

void network::Client::on_closed(CLT_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("OnClosed DEBUG_TRACE %u BytesTransferred:%u @OnClosed\n", sock_ctx->_DEBUG_TRACE, sock_ctx->m_bytes_transferred);
#endif
}

bool network::Client::_init_completion_port() {
    m_completion_port = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (ULONG_PTR)NULL, 0);
    if (m_completion_port == NULL) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Create CompletionPort @_InitCompletionPort\n");
#endif
        return false;
    }

    unsigned int worker_threads_num = m_client_config.o0_Worker_threads > 0 ? m_client_config.o0_Worker_threads : (m_client_config.o0_worker_threads_per_processor * _GetProcessorNum());
    if (worker_threads_num <= 0) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Invalid Worker Threads Number @_InitCompletionPort\n");
#endif
        return false;
    }

    HANDLE* worker_threads = new HANDLE[worker_threads_num];
    memset(worker_threads, 0, sizeof(HANDLE)*worker_threads_num);
    DWORD ThreadId;
    for (unsigned int i = 0; i < worker_threads_num; ++i) {
        WORKER_PARAMS<Client> *worker_params = new WORKER_PARAMS<Client>();
        worker_params->m_instance = this;
        worker_params->m_thread_num = i;

        worker_threads[i] = CreateThread(NULL, 0, ClientWorkThread, worker_params, 0, &ThreadId);
        if (worker_threads[i] == NULL) {
#if(DEBUG&DEBUG_LOG)
            LOG(CC_RED, "Faild to Start Thread%u @_InitCompletionPort\n", i);
#endif
        }
    }
    return true;
}

SOCKET network::Client::_init_sock(unsigned int *port) {
    //SOCKET
    SOCKET sockid = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (sockid == INVALID_SOCKET) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Create Socket @_InitSock\n");
#endif
        return -3;
    }

    SOCKADDR_IN local_addr;
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.S_un.S_addr = INADDR_ANY;
    local_addr.sin_port = htons(port ? (short)*port : 0);

    //BIND
    if (bind(sockid, (SOCKADDR*)&local_addr, sizeof(SOCKADDR)) == SOCKET_ERROR) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Bind Socket @_InitSock\n");
#endif
        return -4;
    }

    if (CreateIoCompletionPort((HANDLE)sockid, m_completion_port, (ULONG_PTR)NULL, 0) == NULL) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Bind Socket with CompletionPort @_InitSock\n");
#endif
        return -5;
    }

    if (!m_pConnectEx) {
        //pACCEPTEX pGETACCEPTEXSOCKADDRS
        GUID GuidConnectEx = WSAID_CONNECTEX;
        DWORD dwBytes = 0;
        if (SOCKET_ERROR == WSAIoctl(
            sockid,
            SIO_GET_EXTENSION_FUNCTION_POINTER,
            &GuidConnectEx,
            sizeof(GuidConnectEx),
            &m_pConnectEx,
            sizeof(m_pConnectEx),
            &dwBytes,
            NULL,
            NULL)) {
#if(DEBUG&DEBUG_LOG)
            LOG(CC_RED, "Faild to Get ConnectEx @_InitSock\n");
#endif
            return -6;
        }
    }

    *port = local_addr.sin_port;

    return sockid;
}

bool network::Client::_post_connect(SOCKET sockid, unsigned long ip, unsigned int port) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("PostConnect @_PostConnect\n");
#endif

    SOCKADDR_IN addr;
    memset(&addr, 0, sizeof(SOCKADDR_IN));
    addr.sin_family = AF_INET;
    addr.sin_addr.S_un.S_addr = ip;
    addr.sin_port = htons(port);

    CLT_SOCKET_CONTEXT *sock_ctx = new CLT_SOCKET_CONTEXT();

    sock_ctx->m_op_type = CLT_OP::CLTOP_CONNECTING;

    DWORD dwBytes = 0;
    if (m_pConnectEx(sockid, (SOCKADDR*)&addr, sizeof(addr), NULL, 0, &dwBytes, (LPOVERLAPPED)sock_ctx) == false) {
        if (WSAGetLastError() != ERROR_IO_PENDING) {
#if(DEBUG&DEBUG_LOG)
            LOG(CC_RED, "Faild to Post Connect%d @_PostConnect\n");
#endif
            return false;
        }
    }

    return true;
}

bool network::Client::_post_send(CLT_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("PostSend DEBUG_TRACE %u @_PostSend\n", sock_ctx->_DEBUG_TRACE);
#endif

    /*if (sock_ctx->m_wsaBuf.len == 0) {
        return true;
    }*/

    DWORD bytes = 0;
    DWORD flags = 0;

    sock_ctx->m_op_type = CLT_OP::CLTOP_SENDING;

    if (WSASend(m_sockid, &(sock_ctx->m_wsa_buf), 1, &bytes, flags, &(sock_ctx->m_OVERLAPPED), NULL) == SOCKET_ERROR) {
        if (WSAGetLastError() != WSA_IO_PENDING) {
#if(DEBUG&DEBUG_LOG)
            LOG(CC_RED, "Faild to Post Send %d @_PostSend\n", WSAGetLastError());
#endif
            return false;
        }
    }

    return true;
}

bool network::Client::_post_recv(CLT_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("PostRecv DEBUG_TRACE %u @_PostRecv\n", sock_ctx->_DEBUG_TRACE);
#endif

    DWORD bytes = 0;
    DWORD flags = 0;

    sock_ctx->m_op_type = CLT_OP::CLTOP_RECVING;

    if (WSARecv(m_sockid, &(sock_ctx->m_wsa_buf), 1, &bytes, &flags, &(sock_ctx->m_OVERLAPPED), NULL) == SOCKET_ERROR) {
        if (WSAGetLastError() != WSA_IO_PENDING) {
#if(DEBUG&DEBUG_LOG)
            LOG(CC_RED, "Faild to Post Recv @_PostRecv\n");
#endif
            return false;
        }
    }

    return true;
}

bool network::Client::_do_connected(CLT_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("DoConnected DEBUG_TRACE %u @_DoConnected\n", sock_ctx->_DEBUG_TRACE);
#endif


    //sock_ctx->RESET_BUFFER();

    if (_post_recv(sock_ctx) == false) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Post Recv @_DoConnected\n");
#endif
        return false;
    }

    on_connected(sock_ctx);

    //sock_ctx->RESET_BUFFER();

    return true;
}

bool network::Client::_do_sent(CLT_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("DoSent DEBUG_TRACE %u @_DoSent\n", sock_ctx->_DEBUG_TRACE);
#endif

    on_sent(sock_ctx);

    delete sock_ctx;//TODO use repeatedly

    return false;
}

bool network::Client::_do_recvd(CLT_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("DoRecvd DEBUG_TRACE %u @_DoRecvd\n", sock_ctx->_DEBUG_TRACE);
#endif

    on_recvd(sock_ctx);

    _post_recv(sock_ctx);

    return false;
}

DWORD network::Client::ClientWorkThread(LPVOID lpParam) {
    WORKER_PARAMS<Client> *worker_params = (WORKER_PARAMS<Client>*)lpParam;

#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("ClientWorkThread ThreadNo:%u @ClientWorkThread\n", worker_params->m_thread_num);
#endif

    DWORD bytes_transferred;
    OVERLAPPED *overlapped;
    CLT_SOCKET_CONTEXT *sock_ctx;
    HANDLE completion_port = worker_params->m_instance->m_completion_port;
    Client *client = worker_params->m_instance;

    typedef void(*DoEvent)(Client*, CLT_SOCKET_CONTEXT*);
    typedef void(*OnEvent)(Client*, CLT_SOCKET_CONTEXT*);

    DoEvent _DoConnected = pointer_cast<DoEvent>(&(Client::_do_connected));
    DoEvent _DoSent = pointer_cast<DoEvent>(&(Client::_do_sent));
    DoEvent _DoRecvd = pointer_cast<DoEvent>(&(Client::_do_recvd));

    OnEvent _OnClosed = pointer_cast<OnEvent>(&Client::on_closed);

    BOOL ret;

    DWORD err_code;

    while (true) {
        bytes_transferred = 0;

        ret = GetQueuedCompletionStatus(completion_port, &bytes_transferred, (PULONG_PTR)&sock_ctx, (LPOVERLAPPED*)&overlapped, INFINITE);

        if (ret == false) {
#if(DEBUG&DEBUG_LOG)
            LOG(CC_RED, "GetQueuedCompletionStatus Failed @ClientWorkThread\n");
#endif

            err_code = GetLastError();

            if (err_code == WAIT_TIMEOUT) {
                if (!_is_server_alive(client->m_sockid)) {
#if(DEBUG&DEBUG_LOG)
                    LOG(CC_YELLOW, "Server Error Socket:%lld @ClientWorkThread\n", client->m_sockid);
#endif
                    _OnClosed(client, sock_ctx);

                    closesocket(client->m_sockid);

                    delete sock_ctx;
                }
                continue;
            } else if (err_code == ERROR_NETNAME_DELETED) {
                if (sock_ctx) {
#if(DEBUG&DEBUG_LOG)
                    LOG(CC_YELLOW, "Server Error Socket:%lld @ClientWorkThread\n", client->m_sockid);
#endif
                    _OnClosed(client, sock_ctx);

                    closesocket(client->m_sockid);

                    delete sock_ctx;
                }

                continue;
            } else if (err_code == ERROR_ABANDONED_WAIT_0) {
                // TODO
                continue;
            } else {
                // TODO
                break;
            }
        }

        sock_ctx = CONTAINING_RECORD(overlapped, CLT_SOCKET_CONTEXT, m_OVERLAPPED);

        if (bytes_transferred == 0 && sock_ctx->m_op_type != CLT_OP::CLTOP_CONNECTING) {
#if(DEBUG&DEBUG_LOG)
            LOG(CC_YELLOW, "Server Offline Socket:%lld @ClientWorkThread\n", client->m_sockid);
#endif
            _OnClosed(client, sock_ctx);

            closesocket(client->m_sockid);

            delete sock_ctx;

            continue;
        }

        sock_ctx->m_bytes_transferred = bytes_transferred;

        switch (sock_ctx->m_op_type) {
        case CLT_OP::CLTOP_CONNECTING:
            _DoConnected(client, sock_ctx);
            break;
        case CLT_OP::CLTOP_SENDING:
            _DoSent(client, sock_ctx);
            break;
        case CLT_OP::CLTOP_RECVING:
            _DoRecvd(client, sock_ctx);
            break;
        default:
            break;
        }
    }

    return 0;
}

bool network::Client::_is_server_alive(SOCKET sockid) {
    int bytes_sent = ::send(sockid, "", 0, 0);

    return bytes_sent != -1;
}

network::Client::~Client() {
    //closesocket(m_Socket);//TODO duplicated with Close()
}