#include "network.h"

#define CLOSE_SOCKET(SOCKID) {if(SOCKID!=INVALID_SOCKET){closesocket(SOCKID);}}

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
    if (m_buffer) {
        delete[] m_buffer;
    }
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("SSC Dispose, DEBUG_TRACE:%u Sockid:%lld Op:%d\n", _DEBUG_TRACE, m_client_sockid, m_op_type);
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
network::Server::Server() :
    m_completion_port(NULL),
    m_sockid(INVALID_SOCKET),
    m_pAcceptEx(NULL),
    m_pGetAcceptExSockAddrs(NULL),
    m_shutdown_event(NULL),
    m_worker_threads(NULL),
    m_worker_threads_num(0) {
}

network::Server::Server(const ServerConfig &serverconfig) :
    m_completion_port(NULL),
    m_sockid(INVALID_SOCKET),
    m_pAcceptEx(NULL),
    m_pGetAcceptExSockAddrs(NULL),
    m_shutdown_event(NULL),
    m_worker_threads(NULL),
    m_worker_threads_num(0) {
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

int network::Server::close_connection(SOCKET sockid) {
#if(DEBUG&DEBUG_LOG)
    LOG(CC_YELLOW, "Close Connection, Sockid:%lld @close_connection\n", sockid);
#endif

    return closesocket(sockid);
}

int network::Server::close_listen() {
#if(DEBUG&DEBUG_LOG)
    LOG(CC_YELLOW, "Close listen, Sockid %d @close_listen\n", m_sockid);
#endif

    int ret = 0;
    if (m_sockid != INVALID_SOCKET) {
        ret = closesocket(m_sockid);
        m_sockid = INVALID_SOCKET;
    }

    return ret;
}

bool network::Server::notify_worker_threads_to_exit() {
#if(DEBUG&DEBUG_LOG)
    LOG(CC_YELLOW, "Notify Worker Threads to Exit, ThreadsNum:%u @close_listen\n", m_worker_threads_num);
#endif
    SetEvent(m_shutdown_event);

    for (unsigned int i = 0; i < m_worker_threads_num + 1; ++i) {
        PostQueuedCompletionStatus(m_completion_port, 0, NULL, NULL);
    }

    return true;
}

bool network::Server::close() {
#if(DEBUG&DEBUG_LOG)
    LOG(CC_YELLOW, "Close Server, Sockid:%lld @close_listen\n", m_sockid);
#endif
    close_listen();

    notify_worker_threads_to_exit();

#if(DEBUG&DEBUG_LOG)
    LOG(CC_YELLOW, "Wait For Worker Threads to Exit, ThreadsNum:%u @close\n", m_worker_threads_num);
#endif
    WaitForMultipleObjects(m_worker_threads_num, m_worker_threads, true, INFINITE);

    if (m_worker_threads) {
        delete[] m_worker_threads;
        m_worker_threads = NULL;
    }

    //TODO errcheck & CRITICAL_SECTION

#if(DEBUG&DEBUG_LOG)
    LOG(CC_YELLOW, "Close Completion Port, CompletionPort:%d @close\n", m_completion_port);
#endif
    if (m_completion_port) {
        CloseHandle(m_completion_port);
        m_completion_port = NULL;
    }
    return true;
}

bool network::Server::_init_complition_port() {
    //Completion Port
    m_completion_port = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (m_completion_port == NULL) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Create Complition Port @_init_complition_port\n");
#endif
        return false;
    }

    m_worker_threads_num = m_server_config.o0_worker_threads > 0 ? m_server_config.o0_worker_threads : (get_processor_num()*m_server_config.o0_worker_threads_per_processor);
    if (m_worker_threads_num <= 0) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Invalid Worker Threads Number @_init_complition_port\n");
#endif
        return false;
    }

    m_worker_threads = new HANDLE[m_worker_threads_num];

    DWORD ThreadId;
    for (unsigned int i = 0; i < m_worker_threads_num; ++i) {
        WORKER_PARAMS<Server> *_pWorkerParams = new WORKER_PARAMS<Server>();
        _pWorkerParams->m_instance = this;
        _pWorkerParams->m_thread_num = i;

        m_worker_threads[i] = CreateThread(NULL, 0, ServerWorkerThread, _pWorkerParams, 0, &ThreadId);
#if(DEBUG&DEBUG_LOG)
        if (m_worker_threads[i] == NULL) {
            LOG(CC_RED, "Failed to Start Thread, ThreadNum:%u @_init_complition_port\n", i);
        }
#endif
    }

    return true;
}

bool network::Server::_init_sock(unsigned int port, unsigned int max_connect) {
    //WSADATA
    WSADATA Wsadata;
    WORD wVersionRequested = MAKEWORD(2, 2);
    if (WSAStartup(wVersionRequested, &Wsadata) != 0) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Load WSAStartup @_init_sock\n");
#endif
        return false;
    }

    if (LOBYTE(Wsadata.wVersion) != 2 || HIBYTE(Wsadata.wVersion) != 2) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Wrong WSA Version(!2.2) @_init_sock\n");
#endif
        return false;
    }

    //SOCKET
    m_sockid = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (m_sockid == INVALID_SOCKET) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Create Socket @_init_sock\n");
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
        LOG(CC_RED, "Faild to Bind Socket @_init_sock\n");
#endif
        return false;
    }

    //LISTEN
    if (listen(m_sockid, max_connect) == SOCKET_ERROR) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Listen Port %d @_init_sock\n", port);
#endif
        return false;
    }

    //COMPLETIONPORT
    if (CreateIoCompletionPort((HANDLE)m_sockid, m_completion_port, (ULONG_PTR)NULL, 0) == NULL) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Bind Socket with CompletionPort @_init_sock\n");
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
        LOG(CC_RED, "Faild to Get AcceptEx @_init_sock\n");
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
        LOG(CC_RED, "Faild to Get GetAcceptExSockAddrs @_init_sock\n");
#endif
        return false;
    }

    //POST ACCEPT
    for (unsigned int i = 0; i < m_server_config.o_max_post_accept; ++i) {
        SVR_SOCKET_CONTEXT *sock_ctx = new SVR_SOCKET_CONTEXT();
        if (_post_accept(sock_ctx) == false) {
#if(DEBUG&DEBUG_LOG)
            LOG(CC_RED, "Faild to Post Accept, AcceptNum:%d @_init_sock\n", i);
#endif
            return false;
        }
    }

    return true;
}

network::Server::~Server() {
    close();
}

bool network::Server::_start(unsigned int port, unsigned int max_connect) {
    if (_init_complition_port() == false) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Init ComplitionPort @_start\n");
#endif
        return false;
    }

    if (_init_sock(port, max_connect) == false) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Init Socket @_start\n");
#endif
        return false;
    }

    return true;
}

bool network::Server::_post_accept(SVR_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("Post Accept, DEBUG_TRACE:%u\n", sock_ctx->_DEBUG_TRACE);
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
            LOG(CC_RED, "Faild to Post Accept, Sockid:%lld @_post_accept\n", sock_ctx->m_client_sockid);
#endif
            return false;
        }
    }

    return true;
}

bool network::Server::_post_recv(SVR_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("Post Recv, DEBUG_TRACE:%u\n", sock_ctx->_DEBUG_TRACE);
#endif

    DWORD bytes = 0;
    DWORD flags = 0;

    sock_ctx->m_op_type = SVR_OP::SVROP_RECVING;

    if (WSARecv(sock_ctx->m_client_sockid, &(sock_ctx->m_wsa_buf), 1, &bytes, &flags, &(sock_ctx->m_OVERLAPPED), NULL) == SOCKET_ERROR &&
        WSAGetLastError() != WSA_IO_PENDING) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Post Recv, Sockid:%lld @_post_recv\n", sock_ctx->m_client_sockid);
#endif
        return false;
    }

    return true;
}

bool network::Server::_post_send(SVR_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("Post Send, DEBUG_TRACE:%u\n", sock_ctx->_DEBUG_TRACE);
#endif

    DWORD bytes = 0;
    DWORD flags = 0;

    sock_ctx->m_op_type = SVR_OP::SVROP_SENDING;

    if (WSASend(sock_ctx->m_client_sockid, &(sock_ctx->m_wsa_buf), 1, &bytes, flags, &(sock_ctx->m_OVERLAPPED), NULL) == SOCKET_ERROR &&
        WSAGetLastError() != WSA_IO_PENDING) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Post Send, Sockid:%lld @_post_send\n", sock_ctx->m_client_sockid);
#endif
        return false;
    }

    return true;
}

void network::Server::_do_accepted(SVR_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("Do Accepted, DEBUG_TRACE:%u Sockid:%d @_do_accepted\n", sock_ctx->_DEBUG_TRACE, sock_ctx->m_client_sockid);
#endif

#if(DEBUG&DEBUG_LOG)
    LOG(CC_YELLOW, "Accepted a Client Sockid:%lld @_do_accepted\n", sock_ctx->m_client_sockid);
#endif

    event_handler(sock_ctx, EVENT_ACCEPTED);

    SVR_SOCKET_CONTEXT *new_sock_ctx = new SVR_SOCKET_CONTEXT();
    new_sock_ctx->m_op_type = SVR_OP::SVROP_RECVING;
    new_sock_ctx->m_client_sockid = sock_ctx->m_client_sockid;
    new_sock_ctx->m_extra = sock_ctx->m_extra;

    //TODO save clientaddr
    //    SOCKADDR_IN *client_addr, *local_addr;//mark:need to delete
    //    int client_addr_len = sizeof(SOCKADDR_IN), local_addr_len = sizeof(SOCKADDR_IN);
    //
    //    m_pGetAcceptExSockAddrs(
    //        sock_ctx->m_wsa_buf.buf,
    //#if(FEATURE_RECV_ON_ACCEPT)
    //        sock_ctx->m_wsaBuf.len - ((sizeof(SOCKADDR_IN) + 16) * 2),
    //#else
    //        0,
    //#endif
    //        sizeof(SOCKADDR_IN) + 16,
    //        sizeof(SOCKADDR_IN) + 16,
    //        (LPSOCKADDR*)&local_addr,
    //        &local_addr_len,
    //        (LPSOCKADDR*)&client_addr,
    //        &client_addr_len);//mark:?
    //
    //    new_sock_ctx->m_client_addr = *client_addr;//mark

#if(DEBUG&DEBUG_LOG)
    //post accept anyway
    if (_post_accept(sock_ctx) == false) {
        LOG(CC_RED, "Faild to Post Accept, Sockid:%lld @_do_accepted\n", sock_ctx->m_client_sockid);
    }
    if (CreateIoCompletionPort((HANDLE)new_sock_ctx->m_client_sockid, m_completion_port, (ULONG_PTR)new_sock_ctx, 0) == NULL) {
        LOG(CC_RED, "Faild to Bind Socket with the Completion Port, Sockid:%lld @_do_accepted\n", sock_ctx->m_client_sockid);
    }
    if (_post_recv(new_sock_ctx) == false) {
        LOG(CC_RED, "Faild to Post Recv, Sockid:%lld @_do_accepted\n", new_sock_ctx->m_client_sockid);
    }
#else
    //post accept anyway
    _post_accept(sock_ctx);

    CreateIoCompletionPort((HANDLE)new_sock_ctx->m_client_sockid, m_completion_port, (ULONG_PTR)new_sock_ctx, 0);

    _post_recv(sock_ctx);
#endif
}

void network::Server::_do_recvd(SVR_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("Do Recvd, DEBUG_TRACE:%u @_do_recvd\n", sock_ctx->_DEBUG_TRACE);
#endif

    event_handler(sock_ctx, EVENT_RECEIVED);

    sock_ctx->m_op_type = SVR_OP::SVROP_RECVING;

#if(DEBUG&DEBUG_LOG)
    if (_post_recv(sock_ctx) == false) {
        LOG(CC_RED, "Faild to Post Recv, Sockid:%lld @_do_recvd\n", sock_ctx->m_client_sockid);
    }
#else
    _post_recv(sock_ctx);
#endif
}

void network::Server::_do_sent(SVR_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("Do Sent, DEBUG_TRACE:%u @_do_sent\n", sock_ctx->_DEBUG_TRACE);
#endif

    event_handler(sock_ctx, EVENT_SENT);

    delete sock_ctx;//TODO use repeatedly
}

void network::Server::_do_closed(SVR_SOCKET_CONTEXT* sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("Do Closed, DEBUG_TRACE:%u @_do_closed\n", sock_ctx->_DEBUG_TRACE);
#endif

    event_handler(sock_ctx, EVENT_CLOSED);

    closesocket(sock_ctx->m_client_sockid);

    delete sock_ctx;
}

bool network::Server::_is_client_alive(SOCKET sockid) {
    int bytes_sent = ::send(sockid, "", 0, 0);

    return bytes_sent != -1;
}

DWORD WINAPI network::Server::ServerWorkerThread(LPVOID lpParam) {
    WORKER_PARAMS<Server> *worker_params = (WORKER_PARAMS<Server>*)lpParam;

#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("ServerWorkThread Start, ThreadNum:%u @ServerWorkerThread\n", worker_params->m_thread_num);
#endif

    DWORD bytes_transferred;
    OVERLAPPED *overlapped;
    SVR_SOCKET_CONTEXT *sock_ctx;
    HANDLE completion_port = worker_params->m_instance->m_completion_port;
    Server *server = worker_params->m_instance;

    BOOL ret;
    DWORD err_code;

    while (true) {
        bytes_transferred = 0;

        ret = GetQueuedCompletionStatus(completion_port, &bytes_transferred, (PULONG_PTR)&sock_ctx, (LPOVERLAPPED*)&overlapped, INFINITE);

        if (ret == false/* && _Overlapped == NULL*/) {
#if(DEBUG&DEBUG_LOG)
            LOG(CC_RED, "GetQueuedCompletionStatus Failed @ServerWorkerThread\n");
#endif

            err_code = GetLastError();

            if (err_code == WAIT_TIMEOUT) {
                if (!_is_client_alive(sock_ctx->m_client_sockid)) {
#if(DEBUG&DEBUG_LOG)
                    LOG(CC_YELLOW, "Client Offline, ErrorCode:%ld Sockid:%lld @ServerWorkerThread\n", WAIT_TIMEOUT, sock_ctx->m_client_sockid);
#endif
                    server->_do_closed(sock_ctx);
                }
                continue;
            } else if (err_code == ERROR_NETNAME_DELETED) {
                if (sock_ctx) {
#if(DEBUG&DEBUG_LOG)
                    LOG(CC_YELLOW, "Client Offline, ErrorCode:%ld Sockid:%lld @ServerWorkerThread\n", ERROR_NETNAME_DELETED, sock_ctx->m_client_sockid);
#endif
                    server->_do_closed(sock_ctx);
                } else {
                    // TODO
                }
            } else if (err_code == ERROR_ABANDONED_WAIT_0) {
                // TODO
                continue;
            } else {
                // TODO
#if(DEBUG&DEBUG_LOG)
                LOG(CC_YELLOW, "Unkwon Error Occur, ErrCode:%d ThreadNum:%d @ServerWorkerThread\n", err_code, worker_params->m_thread_num);
#endif
                break;
            }

            continue;
        }

        if (bytes_transferred == 0 && sock_ctx == NULL && overlapped == NULL) {
#if(DEBUG&DEBUG_LOG)
            LOG(CC_YELLOW, "Exiting Notify Received, ThreadNum:%d @ServerWorkerThread\n", worker_params->m_thread_num);
#endif
            break;
        }

        sock_ctx = CONTAINING_RECORD(overlapped, SVR_SOCKET_CONTEXT, m_OVERLAPPED);

        if (bytes_transferred == 0 && sock_ctx->m_op_type != SVR_OP::SVROP_ACCEPTING) {
#if(DEBUG&DEBUG_LOG)
            LOG(CC_YELLOW, "Zero Bytes Transferred, Sockid:%lld @ServerWorkerThread\n", sock_ctx->m_client_sockid);
#endif
            server->_do_closed(sock_ctx);

            continue;
        }

        sock_ctx->m_bytes_transferred = bytes_transferred;

        switch (sock_ctx->m_op_type) {
        case SVR_OP::SVROP_ACCEPTING:
            server->_do_accepted(sock_ctx);
            break;
        case SVR_OP::SVROP_RECVING:
            server->_do_recvd(sock_ctx);
            break;
        case SVR_OP::SVROP_SENDING:
            server->_do_sent(sock_ctx);
            break;
        default:
            break;
        }
    }

#if(DEBUG&DEBUG_LOG)
    LOG(CC_YELLOW, "Work Thread Exit, ThreadNum:%d @ServerWorkerThread\n", worker_params->m_thread_num);
#endif
    delete worker_params;

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
    if (m_buffer) {
        delete[] m_buffer;
    }
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("CSC Dispose, DEBUG_TRACE:%d\n", _DEBUG_TRACE);
#endif
}

/*
 * ClientConfig
 */
network::ClientConfig::ClientConfig() :
    o0_worker_threads_per_processor(0),
    o0_worker_threads(2) {
}

/*
 * Client
 */
network::Client::Client() :
    m_completion_port(NULL),
    m_pConnectEx(NULL),
    m_sockid(INVALID_SOCKET),
    m_shutdown_event(NULL),
    m_worker_threads(NULL),
    m_worker_threads_num(0) {
    _init();
}

network::Client::Client(const ClientConfig &client_config) :
    m_completion_port(NULL),
    m_pConnectEx(NULL),
    m_sockid(INVALID_SOCKET),
    m_shutdown_event(NULL),
    m_worker_threads(NULL),
    m_worker_threads_num(0) {
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
        LOG(CC_RED, "Faild to Load WSAStartup @_init\n");
#endif
        return -1;
    }

    if (LOBYTE(Wsadata.wVersion) != 2 || HIBYTE(Wsadata.wVersion) != 2) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Wrong WSA Version(!2.2) @_init\n");
#endif
        return -2;
    }

    if (_init_completion_port() == false) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Init ComplitionPort @_init\n");
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
        LOG(CC_RED, "Faild to Init Socket @connect\n");
#endif
        return -2;
    }

    if (_post_connect(m_sockid, inet_addr(addr), port) == false) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Post Connect @connect\n");
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

int network::Client::close_connection() {
#if(DEBUG&DEBUG_LOG)
    LOG(CC_YELLOW, "Close Connection, Sockid:%lld @close_connection\n", m_sockid);
#endif
    //TODO CRITICAL_SECTION?
    int ret = 0;
    if (m_sockid != INVALID_SOCKET) {
        ret = closesocket(m_sockid);
        m_sockid = INVALID_SOCKET;
    }

    return ret;
}

bool network::Client::notify_worker_threads_to_exit() {
#if(DEBUG&DEBUG_LOG)
    LOG(CC_YELLOW, "Notify Worker Threads to Exit, ThreadNum:%u @notify_worker_threads_to_exit\n", m_worker_threads_num);
#endif
    SetEvent(m_shutdown_event);

    //TODO m_worker_threads_num + 1 -> m_worker_threads_num
    for (unsigned int i = 0; i < m_worker_threads_num + 1; ++i) {
        PostQueuedCompletionStatus(m_completion_port, 0, NULL, NULL);
    }

    return true;
}

bool network::Client::close() {
#if(DEBUG&DEBUG_LOG)
    LOG(CC_YELLOW, "Close Client, Sockid:%lld @close\n", m_sockid);
#endif

    //shutdown(m_sockid, SD_BOTH);

    //LINGER _Linger = { 1,0 };
    //_Linger.l_onoff = 0;
    //setsockopt(m_sockid, SOL_SOCKET, SO_LINGER, (const char *)&_Linger, sizeof(_Linger));

    close_connection();

    notify_worker_threads_to_exit();

#if(DEBUG&DEBUG_LOG)
    LOG(CC_YELLOW, "Wait for Worker Threads to Exit, ThreadNum:%u @close\n", m_worker_threads_num);
#endif

    WaitForMultipleObjects(m_worker_threads_num, m_worker_threads, true, INFINITE);

    if (m_worker_threads) {
        delete[] m_worker_threads;
        m_worker_threads = NULL;
    }

#if(DEBUG&DEBUG_LOG)
    LOG(CC_YELLOW, "Close the Completion Port, CompletionPort:%d @close\n", m_completion_port);
#endif
    if (m_completion_port) {
        CloseHandle(m_completion_port);
        m_completion_port = NULL;
    }

    return true;
}

bool network::Client::_init_completion_port() {
    m_completion_port = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (ULONG_PTR)NULL, 0);
    if (m_completion_port == NULL) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Create Completion Port @_init_completion_port\n");
#endif
        return false;
    }

    m_worker_threads_num = m_client_config.o0_worker_threads > 0 ? m_client_config.o0_worker_threads : (m_client_config.o0_worker_threads_per_processor * get_processor_num());
    if (m_worker_threads_num == 0) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Invalid Worker Threads Number, Value:%u @_init_completion_port\n", m_worker_threads_num);
#endif
        return false;
    }

    m_worker_threads = new HANDLE[m_worker_threads_num];

    DWORD ThreadId;
    for (unsigned int i = 0; i < m_worker_threads_num; ++i) {
        WORKER_PARAMS<Client> *worker_params = new WORKER_PARAMS<Client>();
        worker_params->m_instance = this;
        worker_params->m_thread_num = i;

        m_worker_threads[i] = CreateThread(NULL, 0, ClientWorkerThread, worker_params, 0, &ThreadId);
#if(DEBUG&DEBUG_LOG)
        if (m_worker_threads[i] == NULL) {
            LOG(CC_RED, "Faild to Start Thread, ThreadNum:%u @_init_completion_port\n", i);
        }
#endif
    }
    return true;
}

SOCKET network::Client::_init_sock(unsigned int *port) {
    //SOCKET
    SOCKET sockid = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (sockid == INVALID_SOCKET) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Create Socket @_init_sock\n");
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
        LOG(CC_RED, "Faild to Bind Socket @_init_sock\n");
#endif
        return -4;
    }

    if (CreateIoCompletionPort((HANDLE)sockid, m_completion_port, (ULONG_PTR)NULL, 0) == NULL) {
#if(DEBUG&DEBUG_LOG)
        LOG(CC_RED, "Faild to Bind Socket with CompletionPort @_init_sock\n");
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
            LOG(CC_RED, "Faild to Get ConnectEx @_init_sock\n");
#endif
            return -6;
        }
    }

    *port = local_addr.sin_port;

    return sockid;
}

bool network::Client::_post_connect(SOCKET sockid, unsigned long ip, unsigned int port) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("Post Connect @_post_connect\n");
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
            LOG(CC_RED, "Faild to Post Connect @_post_connect\n");
#endif
            return false;
        }
    }

    return true;
}

bool network::Client::_post_send(CLT_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("Post Send, DEBUG_TRACE:%u @_post_send\n", sock_ctx->_DEBUG_TRACE);
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
            LOG(CC_RED, "Faild to Post Send, ErrorCode:%d @_post_send\n", WSAGetLastError());
#endif
            return false;
        }
    }

    return true;
}

bool network::Client::_post_recv(CLT_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("Post Recv, DEBUG_TRACE:%u @_post_recv\n", sock_ctx->_DEBUG_TRACE);
#endif

    DWORD bytes = 0;
    DWORD flags = 0;

    sock_ctx->m_op_type = CLT_OP::CLTOP_RECVING;

    if (WSARecv(m_sockid, &(sock_ctx->m_wsa_buf), 1, &bytes, &flags, &(sock_ctx->m_OVERLAPPED), NULL) == SOCKET_ERROR) {
        if (WSAGetLastError() != WSA_IO_PENDING) {
#if(DEBUG&DEBUG_LOG)
            LOG(CC_RED, "Faild to Post Recv @_post_recv\n");
#endif
            return false;
        }
    }

    return true;
}

void network::Client::_do_connected(CLT_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("Do Connected, DEBUG_TRACE:%u @_do_connected\n", sock_ctx->_DEBUG_TRACE);
#endif

    event_handler(sock_ctx, EVENT_CONNECTED);

    //sock_ctx->RESET_BUFFER();

#if(DEBUG&DEBUG_LOG)
    if (_post_recv(sock_ctx) == false) {
        LOG(CC_RED, "Faild to Post Recv @_do_connected\n");
    }
#else
    _post_recv(sock_ctx);
#endif
}

void network::Client::_do_sent(CLT_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("Do Sent, DEBUG_TRACE:%u @_do_sent\n", sock_ctx->_DEBUG_TRACE);
#endif

    event_handler(sock_ctx, EVENT_SENT);

    delete sock_ctx;//TODO use repeatedly
}

void network::Client::_do_recvd(CLT_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("Do Recvd, DEBUG_TRACE:%u @_do_recvd\n", sock_ctx->_DEBUG_TRACE);
#endif

    event_handler(sock_ctx, EVENT_RECEIVED);

#if(DEBUG&DEBUG_LOG)
    if (_post_recv(sock_ctx) == false) {
        LOG(CC_RED, "Faild to Post Recv @_do_recvd\n");
    }
#else
    _post_recv(sock_ctx);
#endif
}

void network::Client::_do_closed(CLT_SOCKET_CONTEXT *sock_ctx) {
#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("Do Closed, DEBUG_TRACE:%u @_do_closed\n", sock_ctx->_DEBUG_TRACE);
#endif

    event_handler(sock_ctx, EVENT_CLOSED);

    closesocket(m_sockid);//mark:necessary?
    m_sockid = INVALID_SOCKET;

    delete sock_ctx;
}

DWORD network::Client::ClientWorkerThread(LPVOID lpParam) {
    WORKER_PARAMS<Client> *worker_params = (WORKER_PARAMS<Client>*)lpParam;

#if(DEBUG&DEBUG_TRACE)
    TRACE_PRINT("ClientWorkerThread Start, ThreadNum %u @ClientWorkerThread\n", worker_params->m_thread_num);
#endif

    DWORD bytes_transferred;
    OVERLAPPED *overlapped;
    CLT_SOCKET_CONTEXT *sock_ctx;
    HANDLE completion_port = worker_params->m_instance->m_completion_port;
    Client *client = worker_params->m_instance;

    BOOL ret;
    DWORD err_code;

    while (WaitForSingleObject(client->m_shutdown_event, 0) != WAIT_OBJECT_0) {
        bytes_transferred = 0;

        ret = GetQueuedCompletionStatus(completion_port, &bytes_transferred, (PULONG_PTR)&sock_ctx, (LPOVERLAPPED*)&overlapped, INFINITE);

        //TODO opt close

        if (ret == false) {
#if(DEBUG&DEBUG_LOG)
            LOG(CC_RED, "GetQueuedCompletionStatus Failed @ClientWorkerThread\n");
#endif

            err_code = GetLastError();

            if (err_code == WAIT_TIMEOUT) {
                if (!_is_server_alive(client->m_sockid)) {
#if(DEBUG&DEBUG_LOG)
                    LOG(CC_YELLOW, "Server Offline, Error:WAIT_TIMEOUT Sockid:%lld @ClientWorkerThread\n", client->m_sockid);
#endif
                    client->_do_closed(sock_ctx);
                }
            } else if (err_code == ERROR_NETNAME_DELETED) {
                if (sock_ctx) {
#if(DEBUG&DEBUG_LOG)
                    LOG(CC_YELLOW, "Server Offline, Error:ERROR_NETNAME_DELETED Sockid:%lld @ClientWorkerThread\n", client->m_sockid);
#endif
                    client->_do_closed(sock_ctx);
                }
            } else if (err_code == ERROR_ABANDONED_WAIT_0) {
                // TODO
            } else {
                // TODO
#if(DEBUG&DEBUG_LOG)
                LOG(CC_YELLOW, "Unkwon Error Occur, ErrCode:%d ThreadNum:%d @ClientWorkerThread\n", err_code, worker_params->m_thread_num);
#endif
            }
            break;
        }

        if (bytes_transferred == 0 && sock_ctx == NULL && overlapped == NULL) {
#if(DEBUG&DEBUG_LOG)
            LOG(CC_YELLOW, "Exiting Notify Received, ThreadNum:%d @ClientWorkerThread\n", worker_params->m_thread_num);
#endif
            break;
        }

        sock_ctx = CONTAINING_RECORD(overlapped, CLT_SOCKET_CONTEXT, m_OVERLAPPED);

        if (bytes_transferred == 0 && sock_ctx->m_op_type != CLT_OP::CLTOP_CONNECTING) {
#if(DEBUG&DEBUG_LOG)
            LOG(CC_YELLOW, "Zero Bytes Transferred, Sockid:%lld @ClientWorkerThread\n", client->m_sockid);
#endif
            client->_do_closed(sock_ctx);

            break;
        }

        sock_ctx->m_bytes_transferred = bytes_transferred;

        switch (sock_ctx->m_op_type) {
        case CLT_OP::CLTOP_CONNECTING:
            client->_do_connected(sock_ctx);
            break;
        case CLT_OP::CLTOP_SENDING:
            client->_do_sent(sock_ctx);
            break;
        case CLT_OP::CLTOP_RECVING:
            client->_do_recvd(sock_ctx);
            break;
        default:
            break;
        }
    }

#if(DEBUG&DEBUG_LOG)
    LOG(CC_YELLOW, "Worker Thread Exit, ThreadNum:%d @ClientWorkerThread\n", worker_params->m_thread_num);
#endif
    delete worker_params;

    return 0;
}

bool network::Client::_is_server_alive(SOCKET sockid) {
    int bytes_sent = ::send(sockid, "", 0, 0);

    return bytes_sent != -1;
}

network::Client::~Client() {
    close();
}