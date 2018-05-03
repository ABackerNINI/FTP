#include "ftp_dtp.h"
#include <assert.h>

/*
 *  ftp_dtp_passive_send
 */
ftp_dtp::_ftp_dtp_passive_send::_ftp_dtp_passive_send(const char *fpath, unsigned int *local_port, HANDLE completion_port_2, SOCKET sockid_2) :
    m_completion_port_2(completion_port_2),
    m_sockid_2(sockid_2) {
    if (!ftp_dtp::_server_config) {
        ftp_dtp::_server_config = new network::ServerConfig();
        ftp_dtp::_server_config->o0_worker_threads = 2;
        ftp_dtp::_server_config->o_max_post_accept = 1;
        ftp_dtp::_server_config->o_max_connect = 1;
    }
    if (m_file.open(fpath, "rb") != NULL) {
        network::Server::set_config(*ftp_dtp::_server_config);
        network::Server::start_listen(local_port);
    }
}

bool ftp_dtp::_ftp_dtp_passive_send::abort() {
    network::Server::close();

    return true;
}

void ftp_dtp::_ftp_dtp_passive_send::event_handler(network::SVR_SOCKET_CONTEXT *sock_ctx, int ev) {
    switch (ev) {
    case EVENT_CONNECTED:
        _on_connected(sock_ctx);
        break;
    case EVENT_RECEIVED:
        _on_recvd(sock_ctx);
        break;
    case EVENT_SENT:
        _on_sent(sock_ctx);
        break;
    case EVENT_CLOSED:
        _on_closed(sock_ctx);
        break;
    default:
        break;
    }
}

void ftp_dtp::_ftp_dtp_passive_send::_on_connected(network::SVR_SOCKET_CONTEXT *sock_ctx) {
    char buffer[DEFAULT_BUFFER_LEN];
    size_t count;
    SOCKET sockid = sock_ctx->m_client_sockid;

    //m_fsize = m_file.size();

    while (!m_file.feof()) {
        count = m_file.read(buffer, sizeof(char), DEFAULT_BUFFER_LEN);

        if (m_file.ferror() || !send(sockid, buffer, count)) {
            break;
        }
        m_bytes_transfered += count;
    }

    m_file.close();

    //close the server step by step
    close_connection(sockid);
    close_listen();
    notify_worker_threads_to_exit();
}

void ftp_dtp::_ftp_dtp_passive_send::_on_sent(network::SVR_SOCKET_CONTEXT *sock_ctx) {
}

void ftp_dtp::_ftp_dtp_passive_send::_on_recvd(network::SVR_SOCKET_CONTEXT *sock_ctx) {
}

void ftp_dtp::_ftp_dtp_passive_send::_on_closed(network::SVR_SOCKET_CONTEXT *sock_ctx) {
    m_file.close();

    //close the server step by step
    close_connection(sock_ctx->m_client_sockid);
    close_listen();
    notify_worker_threads_to_exit();

    //post done-msg back
    if (m_completion_port_2) {
        PostQueuedCompletionStatus(m_completion_port_2, EVENT_USER_FIRST, (ULONG_PTR)&m_sockid_2, NULL);
    }

    printf("done\n");
}

/*
 *  ftp_dtp_passive_recv
*/
ftp_dtp::_ftp_dtp_passive_recv::_ftp_dtp_passive_recv(const char *fpath, unsigned int *local_port, HANDLE completion_port_2, SOCKET sockid_2) :
    m_completion_port_2(completion_port_2),
    m_sockid_2(sockid_2) {
    if (!ftp_dtp::_server_config) {
        ftp_dtp::_server_config = new network::ServerConfig();
        ftp_dtp::_server_config->o0_worker_threads = 2;
        ftp_dtp::_server_config->o_max_post_accept = 1;
        ftp_dtp::_server_config->o_max_connect = 1;
    }
    if (m_file.open(fpath, "wb")) {
        network::Server::set_config(*ftp_dtp::_server_config);
        network::Server::start_listen(0);
    }
}

bool ftp_dtp::_ftp_dtp_passive_recv::abort() {
    network::Server::close();

    return true;
}

void ftp_dtp::_ftp_dtp_passive_recv::event_handler(network::SVR_SOCKET_CONTEXT *sock_ctx, int ev) {
    switch (ev) {
    case EVENT_CONNECTED:
        _on_connected(sock_ctx);
        break;
    case EVENT_RECEIVED:
        _on_recvd(sock_ctx);
        break;
    case EVENT_SENT:
        _on_sent(sock_ctx);
        break;
    case EVENT_CLOSED:
        _on_closed(sock_ctx);
        break;
    default:
        break;
    }
}

void ftp_dtp::_ftp_dtp_passive_recv::_on_connected(network::SVR_SOCKET_CONTEXT *sock_ctx) {
}

void ftp_dtp::_ftp_dtp_passive_recv::_on_sent(network::SVR_SOCKET_CONTEXT *sock_ctx) {
}

void ftp_dtp::_ftp_dtp_passive_recv::_on_recvd(network::SVR_SOCKET_CONTEXT *sock_ctx) {
    m_file.write(sock_ctx->m_buffer, sizeof(char), sock_ctx->m_bytes_transferred);
    m_bytes_transfered += sock_ctx->m_bytes_transferred;
}

void ftp_dtp::_ftp_dtp_passive_recv::_on_closed(network::SVR_SOCKET_CONTEXT *sock_ctx) {
    m_file.close();

    //close the server step by step
    close_connection(sock_ctx->m_client_sockid);
    close_listen();
    notify_worker_threads_to_exit();

    //post done-msg back
    if (m_completion_port_2) {
        PostQueuedCompletionStatus(m_completion_port_2, EVENT_USER_FIRST, (ULONG_PTR)&m_sockid_2, NULL);
    }

    printf("done\n");
}

/*
 *  ftp_dtp_active_send
 */
ftp_dtp::_ftp_dtp_active_send::_ftp_dtp_active_send(const char *fpath, const char *addr, unsigned int port, HANDLE completion_port_2, SOCKET sockid_2) :
    m_completion_port_2(completion_port_2),
    m_sockid_2(sockid_2) {
    if (m_file.open(fpath, "rb")) {
        network::Client::connect(addr, port);
    }
}

bool ftp_dtp::_ftp_dtp_active_send::abort() {
    network::Client::close();

    return true;
}

void ftp_dtp::_ftp_dtp_active_send::event_handler(network::CLT_SOCKET_CONTEXT *sock_ctx, int ev) {
    switch (ev) {
    case EVENT_CONNECTED:
        _on_connected(sock_ctx);
        break;
    case EVENT_RECEIVED:
        _on_recvd(sock_ctx);
        break;
    case EVENT_SENT:
        _on_sent(sock_ctx);
        break;
    case EVENT_CLOSED:
        _on_closed(sock_ctx);
        break;
    default:
        break;
    }
}

void ftp_dtp::_ftp_dtp_active_send::_on_connected(network::CLT_SOCKET_CONTEXT *sock_ctx) {
    char buffer[DEFAULT_BUFFER_LEN];
    size_t count;

    //m_fsize = m_file.size();

    while (!m_file.feof()) {
        count = m_file.read(buffer, sizeof(char), DEFAULT_BUFFER_LEN);

        if (m_file.ferror() || !send(buffer, count)) {
            break;
        }
        m_bytes_transfered += count;
    }

    m_file.close();

    //close the client step by step
    close_connection();
    notify_worker_threads_to_exit();
}

void ftp_dtp::_ftp_dtp_active_send::_on_sent(network::CLT_SOCKET_CONTEXT *sock_ctx) {
}

void ftp_dtp::_ftp_dtp_active_send::_on_recvd(network::CLT_SOCKET_CONTEXT *sock_ctx) {
}

void ftp_dtp::_ftp_dtp_active_send::_on_closed(network::CLT_SOCKET_CONTEXT *sock_ctx) {
    m_file.close();

    //close the client step by step
    close_connection();
    notify_worker_threads_to_exit();

    //post done-msg back
    if (m_completion_port_2) {
        PostQueuedCompletionStatus(m_completion_port_2, EVENT_USER_FIRST, (ULONG_PTR)&m_sockid_2, NULL);
    }

    printf("done\n");
}

/*
 *  ftp_dtp_active_recv
 */
ftp_dtp::_ftp_dtp_active_recv::_ftp_dtp_active_recv(const char *fpath, const char *addr, unsigned int port, HANDLE completion_port_2, SOCKET sockid_2) :
    m_completion_port_2(completion_port_2),
    m_sockid_2(sockid_2) {
    if (m_file.open(fpath, "wb")) {
        network::Client::connect(addr, port);
    }
}

bool ftp_dtp::_ftp_dtp_active_recv::abort() {
    network::Client::close();

    return true;
}

void ftp_dtp::_ftp_dtp_active_recv::event_handler(network::CLT_SOCKET_CONTEXT *sock_ctx, int ev) {
    switch (ev) {
    case EVENT_CONNECTED:
        _on_connected(sock_ctx);
        break;
    case EVENT_RECEIVED:
        _on_recvd(sock_ctx);
        break;
    case EVENT_SENT:
        _on_sent(sock_ctx);
        break;
    case EVENT_CLOSED:
        _on_closed(sock_ctx);
        break;
    default:
        break;
    }
}

void ftp_dtp::_ftp_dtp_active_recv::_on_connected(network::CLT_SOCKET_CONTEXT *sock_ctx) {
}

void ftp_dtp::_ftp_dtp_active_recv::_on_sent(network::CLT_SOCKET_CONTEXT *sock_ctx) {
}

void ftp_dtp::_ftp_dtp_active_recv::_on_recvd(network::CLT_SOCKET_CONTEXT *sock_ctx) {
    m_file.write(sock_ctx->m_buffer, sizeof(char), sock_ctx->m_bytes_transferred);
    m_bytes_transfered += sock_ctx->m_bytes_transferred;
}

void ftp_dtp::_ftp_dtp_active_recv::_on_closed(network::CLT_SOCKET_CONTEXT *sock_ctx) {
    m_file.close();

    //close the client step by step
    close_connection();
    notify_worker_threads_to_exit();

    //post done-msg back
    if (m_completion_port_2) {
        PostQueuedCompletionStatus(m_completion_port_2, EVENT_USER_FIRST, (ULONG_PTR)&m_sockid_2, NULL);
    }

    printf("done\n");
}

/*
 *  ftp_dtp
 */
ftp_dtp::FtpDtp::FtpDtp() :
    m_dtp(NULL),
    m_dtp_type(FTP_DTP_TYPE_NULL) {
}

bool ftp_dtp::FtpDtp::start(int dtp_type, const char *fpath, const char *addr, unsigned int port, unsigned int *local_port /*= NULL*/, HANDLE completion_port_2 /*= NULL*/, SOCKET sockid_2 /*= INVALID_SOCKET*/) {
    if (!m_dtp) {
        switch (dtp_type) {
        case FTP_DTP_TYPE_PASSIVE_SEND:
            m_dtp = new _ftp_dtp_passive_send(fpath, local_port, completion_port_2, sockid_2);
            break;
        case FTP_DTP_TYPE_PASSIVE_RECV:
            m_dtp = new _ftp_dtp_passive_recv(fpath, local_port, completion_port_2, sockid_2);
            break;
        case FTP_DTP_TYPE_ACTIVE_SEND:
            m_dtp = new _ftp_dtp_active_send(fpath, addr, port, completion_port_2, sockid_2);
            break;
        case FTP_DTP_TYPE_ACTIVE_RECV:
            m_dtp = new _ftp_dtp_active_recv(fpath, addr, port, completion_port_2, sockid_2);
            break;
        default:
            assert(false);
        }
        m_dtp_type = dtp_type;

        return true;
    }

    return false;
}

bool ftp_dtp::FtpDtp::abort() {
    switch (m_dtp_type) {
    case FTP_DTP_TYPE_PASSIVE_SEND:
        ((_ftp_dtp_passive_send *)m_dtp)->abort();
        break;
    case FTP_DTP_TYPE_PASSIVE_RECV:
        ((_ftp_dtp_passive_recv *)m_dtp)->abort();
        break;
    case FTP_DTP_TYPE_ACTIVE_SEND:
        ((_ftp_dtp_active_send *)m_dtp)->abort();
        break;
    case FTP_DTP_TYPE_ACTIVE_RECV:
        ((_ftp_dtp_active_recv *)m_dtp)->abort();
        break;
    default:
        assert(false);
    }

    return true;
}

bool ftp_dtp::FtpDtp::close() {
    if (m_dtp) {
        this->abort();
        delete m_dtp;
        m_dtp = NULL;
        m_dtp_type = FTP_DTP_TYPE_NULL;
    }

    return true;
}

ftp_dtp::FtpDtp::~FtpDtp() {
}
