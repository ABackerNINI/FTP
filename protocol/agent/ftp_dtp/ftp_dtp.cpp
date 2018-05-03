#include "ftp_dtp.h"
#include <assert.h>

/*
 *  ftp_dtp_passive
 */
ftp_dtp::ftp_dtp_active::ftp_dtp_active() :
    m_completion_port_2(NULL),
    m_sockid_2(INVALID_SOCKET) {
}

bool ftp_dtp::ftp_dtp_active::abort() {
    close();

    return true;
}

void ftp_dtp::ftp_dtp_active::set_completion_port_2(HANDLE completion_port_2) {
    m_completion_port_2 = completion_port_2;
}

void ftp_dtp::ftp_dtp_active::set_sockid_2(SOCKET sockid) {
    m_sockid_2 = sockid;
}

void ftp_dtp::ftp_dtp_active::set_is_to_send(bool is_to_send) {
    m_is_to_send = is_to_send;
}

void ftp_dtp::ftp_dtp_active::set_fpath(const char *fpath) { m_fpath = fpath; }

size_t ftp_dtp::ftp_dtp_active::get_bytes_sent() { return m_bytes_transfered; }

size_t ftp_dtp::ftp_dtp_active::get_fsize() { return m_fsize; }

void ftp_dtp::ftp_dtp_active::event_handler(network::CLT_SOCKET_CONTEXT *sock_ctx, int ev) {
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

void ftp_dtp::ftp_dtp_active::_on_connected(network::CLT_SOCKET_CONTEXT *sock_ctx) {
    if (m_is_to_send) {
        char buffer[DEFAULT_BUFFER_LEN];
        size_t count;

        if (m_file.open(m_fpath, "rb") == NULL) {
            return;
        }

        m_fsize = m_file.size();

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
    } else {
        m_file.open(m_fpath, "wb");
    }
}

void ftp_dtp::ftp_dtp_active::_on_sent(network::CLT_SOCKET_CONTEXT *sock_ctx) {
}

void ftp_dtp::ftp_dtp_active::_on_recvd(network::CLT_SOCKET_CONTEXT *sock_ctx) {
    if (!m_is_to_send) {
        m_file.write(sock_ctx->m_buffer, sizeof(char), sock_ctx->m_bytes_transferred);
        m_bytes_transfered += sock_ctx->m_bytes_transferred;
    }
}

void ftp_dtp::ftp_dtp_active::_on_closed(network::CLT_SOCKET_CONTEXT *sock_ctx) {
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
 *  ftp_dtp_passive
 */
ftp_dtp::ftp_dtp_passive::ftp_dtp_passive() :
    m_completion_port_2(NULL),
    m_sockid_2(INVALID_SOCKET) {
}

bool ftp_dtp::ftp_dtp_passive::abort() {
    close();

    return true;
}

void ftp_dtp::ftp_dtp_passive::set_is_to_send(bool is_to_send) {
    m_is_to_send = is_to_send;
}

void ftp_dtp::ftp_dtp_passive::set_fpath(const char *fpath) { m_fpath = fpath; }

void ftp_dtp::ftp_dtp_passive::set_completion_port_2(HANDLE completion_port_2) {
    m_completion_port_2 = completion_port_2;
}

void ftp_dtp::ftp_dtp_passive::set_sockid_2(SOCKET sockid) {
    m_sockid_2 = sockid;
}

size_t ftp_dtp::ftp_dtp_passive::get_bytes_recvd() { return m_bytes_transfered; }

void ftp_dtp::ftp_dtp_passive::event_handler(network::SVR_SOCKET_CONTEXT *sock_ctx, int ev) {
    switch (ev) {
    case EVENT_ACCEPTED:
        _on_accepted(sock_ctx);
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

void ftp_dtp::ftp_dtp_passive::_on_accepted(network::SVR_SOCKET_CONTEXT *sock_ctx) {
    if (m_is_to_send) {
        char buffer[DEFAULT_BUFFER_LEN];
        size_t count;
        SOCKET sockid = sock_ctx->m_client_sockid;

        if (m_file.open(m_fpath, "rb") == NULL) {
            return;
        }

        m_fsize = m_file.size();

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
    } else {
        m_file.open(m_fpath, "wb");
    }
}

void ftp_dtp::ftp_dtp_passive::_on_recvd(network::SVR_SOCKET_CONTEXT *sock_ctx) {
    if (!m_is_to_send) {
        m_file.write(sock_ctx->m_buffer, sizeof(char), sock_ctx->m_bytes_transferred);
        m_bytes_transfered += sock_ctx->m_bytes_transferred;
    }
}

void ftp_dtp::ftp_dtp_passive::_on_sent(network::SVR_SOCKET_CONTEXT *sock_ctx) {
}

void ftp_dtp::ftp_dtp_passive::_on_closed(network::SVR_SOCKET_CONTEXT *sock_ctx) {
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
 *  ftp_dtp
 */
ftp_dtp::ftp_dtp::ftp_dtp() :
    m_completion_port_2(NULL),
    m_sockid_2(INVALID_SOCKET),
    m_passive(false),
    m_is_to_send(false),
    m_dtp_active(NULL),
    m_dtp_passive(NULL) {
}

bool ftp_dtp::ftp_dtp::start() {
    close();

    static network::ServerConfig *server_config;
    if (!server_config) {
        server_config = new network::ServerConfig();
        server_config->o0_worker_threads = 2;
        server_config->o_max_post_accept = 1;
        server_config->o_max_connect = 1;
    }

    if (m_passive) {
        m_dtp_passive = new ftp_dtp_passive();
        m_dtp_passive->set_config(*server_config);
        m_dtp_passive->set_completion_port_2(m_completion_port_2);
        m_dtp_passive->set_sockid_2(m_sockid_2);
        m_dtp_passive->set_is_to_send(m_is_to_send);
        m_dtp_passive->set_fpath(m_fpath);
        m_dtp_passive->start_listen(&m_port);
    } else {
        m_dtp_active = new ftp_dtp_active();
        m_dtp_active->set_completion_port_2(m_completion_port_2);
        m_dtp_active->set_sockid_2(m_sockid_2);
        m_dtp_active->set_is_to_send(m_is_to_send);
        m_dtp_active->set_fpath(m_fpath);
        m_dtp_active->connect(m_addr,m_port);
    }

    return true;
}

bool ftp_dtp::ftp_dtp::abort() {
    return close();
}

bool ftp_dtp::ftp_dtp::close() {
    if (m_dtp_passive) {
        m_dtp_passive->close();
        delete m_dtp_passive;
        m_dtp_passive = NULL;
    }
    if (m_dtp_active) {
        m_dtp_active->close();
        delete m_dtp_active;
        m_dtp_active = NULL;
    }

    return true;
}

bool ftp_dtp::ftp_dtp::get_passive() {
    return m_passive;
}

const char *ftp_dtp::ftp_dtp::get_addr() {
    return m_addr;
}

int ftp_dtp::ftp_dtp::get_port() {
    return m_port;
}

void ftp_dtp::ftp_dtp::set_passive(bool passive) {
    m_passive = passive;
}

void ftp_dtp::ftp_dtp::set_is_to_send(bool is_to_send) {
    m_is_to_send = is_to_send;
}

void ftp_dtp::ftp_dtp::set_fpath(const char *fpath) {
    m_fpath = fpath;
}

void ftp_dtp::ftp_dtp::set_addr(const char *addr) {
    m_addr = addr;
}

void ftp_dtp::ftp_dtp::set_port(unsigned int port) {
    m_port = port;
}

void ftp_dtp::ftp_dtp::set_completion_port_2(HANDLE completion_port_2) {
    m_completion_port_2 = completion_port_2;
}

void ftp_dtp::ftp_dtp::set_sockid_2(SOCKET sockid) {
    m_sockid_2 = sockid;
}

ftp_dtp::ftp_dtp::~ftp_dtp() {
    close();
}


//enum ftp_dtp::STATUS ftp_dtp::ftp_dtp::get_status() {
//    return STATUS(0);
//}
//
//void ftp_dtp::ftp_dtp::set_structure_type(STRUCTURE_TYPE sturcture_type) {
//}
//
//void ftp_dtp::ftp_dtp::set_data_type(DATA_TYPE data_type) {
//}
//enum ftp_dtp::STRUCTURE_TYPE ftp_dtp::ftp_dtp::get_structure_type() {
//    return STRUCTURE_TYPE(0);
//}
//
//enum ftp_dtp::DATA_TYPE ftp_dtp::ftp_dtp::get_data_type() {
//    return DATA_TYPE(0);
//}

/*
 *  ftp_dtp_passive_send
 */
ftp_dtp::ftp_dtp_passive_send::ftp_dtp_passive_send(const char *fpath, unsigned int *local_port, HANDLE completion_port_2, SOCKET sockid_2) :
    m_completion_port_2(completion_port_2),
    m_sockid_2(sockid_2) {
    if (m_file.open(fpath, "rb") != NULL) {
        network::Server::start_listen(local_port);
    }
}

bool ftp_dtp::ftp_dtp_passive_send::abort() {
    network::Server::close();

    return true;
}

void ftp_dtp::ftp_dtp_passive_send::event_handler(network::SVR_SOCKET_CONTEXT *sock_ctx, int ev) {
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

void ftp_dtp::ftp_dtp_passive_send::_on_connected(network::SVR_SOCKET_CONTEXT *sock_ctx) {
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

void ftp_dtp::ftp_dtp_passive_send::_on_sent(network::SVR_SOCKET_CONTEXT *sock_ctx) {
}

void ftp_dtp::ftp_dtp_passive_send::_on_recvd(network::SVR_SOCKET_CONTEXT *sock_ctx) {
}

void ftp_dtp::ftp_dtp_passive_send::_on_closed(network::SVR_SOCKET_CONTEXT *sock_ctx) {
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
ftp_dtp::ftp_dtp_passive_recv::ftp_dtp_passive_recv(const char *fpath, unsigned int *local_port, HANDLE completion_port_2, SOCKET sockid_2) :
    m_completion_port_2(completion_port_2),
    m_sockid_2(sockid_2) {
    if (m_file.open(fpath, "wb")) {
        network::Server::start_listen(0);
    }
}

bool ftp_dtp::ftp_dtp_passive_recv::abort() {
    network::Server::close();

    return true;
}

void ftp_dtp::ftp_dtp_passive_recv::event_handler(network::SVR_SOCKET_CONTEXT *sock_ctx, int ev) {
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

void ftp_dtp::ftp_dtp_passive_recv::_on_connected(network::SVR_SOCKET_CONTEXT *sock_ctx) {
}

void ftp_dtp::ftp_dtp_passive_recv::_on_sent(network::SVR_SOCKET_CONTEXT *sock_ctx) {
}

void ftp_dtp::ftp_dtp_passive_recv::_on_recvd(network::SVR_SOCKET_CONTEXT *sock_ctx) {
    m_file.write(sock_ctx->m_buffer, sizeof(char), sock_ctx->m_bytes_transferred);
    m_bytes_transfered += sock_ctx->m_bytes_transferred;
}

void ftp_dtp::ftp_dtp_passive_recv::_on_closed(network::SVR_SOCKET_CONTEXT *sock_ctx) {
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
ftp_dtp::ftp_dtp_active_send::ftp_dtp_active_send(const char *fpath, const char *addr, unsigned int port, HANDLE completion_port_2, SOCKET sockid_2) :
    m_completion_port_2(completion_port_2),
    m_sockid_2(sockid_2) {
    if (m_file.open(fpath, "rb")) {
        network::Client::connect(addr, port);
    }
}

bool ftp_dtp::ftp_dtp_active_send::abort() {
    network::Client::close();

    return true;
}

void ftp_dtp::ftp_dtp_active_send::event_handler(network::CLT_SOCKET_CONTEXT *sock_ctx, int ev) {
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

void ftp_dtp::ftp_dtp_active_send::_on_connected(network::CLT_SOCKET_CONTEXT *sock_ctx) {
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

void ftp_dtp::ftp_dtp_active_send::_on_sent(network::CLT_SOCKET_CONTEXT *sock_ctx) {
}

void ftp_dtp::ftp_dtp_active_send::_on_recvd(network::CLT_SOCKET_CONTEXT *sock_ctx) {
}

void ftp_dtp::ftp_dtp_active_send::_on_closed(network::CLT_SOCKET_CONTEXT *sock_ctx) {
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
ftp_dtp::ftp_dtp_active_recv::ftp_dtp_active_recv(const char *fpath, const char *addr, unsigned int port, HANDLE completion_port_2, SOCKET sockid_2) :
    m_completion_port_2(completion_port_2),
    m_sockid_2(sockid_2) {
    if (m_file.open(fpath, "wb")) {
        network::Client::connect(addr, port);
    }
}

bool ftp_dtp::ftp_dtp_active_recv::abort() {
    network::Client::close();

    return true;
}

void ftp_dtp::ftp_dtp_active_recv::event_handler(network::CLT_SOCKET_CONTEXT *sock_ctx, int ev) {
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

void ftp_dtp::ftp_dtp_active_recv::_on_connected(network::CLT_SOCKET_CONTEXT *sock_ctx) {
}

void ftp_dtp::ftp_dtp_active_recv::_on_sent(network::CLT_SOCKET_CONTEXT *sock_ctx) {
}

void ftp_dtp::ftp_dtp_active_recv::_on_recvd(network::CLT_SOCKET_CONTEXT *sock_ctx) {
    m_file.write(sock_ctx->m_buffer, sizeof(char), sock_ctx->m_bytes_transferred);
    m_bytes_transfered += sock_ctx->m_bytes_transferred;
}

void ftp_dtp::ftp_dtp_active_recv::_on_closed(network::CLT_SOCKET_CONTEXT *sock_ctx) {
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
ftp_dtp::ftp_dtp_2::ftp_dtp_2() :
    m_dtp(NULL),
    m_dtp_type(FTP_DTP_TYPE_NULL) {
}

bool ftp_dtp::ftp_dtp_2::start(int dtp_type, const char *fpath, const char *addr, unsigned int port, unsigned int *local_port /*= NULL*/, HANDLE completion_port_2 /*= NULL*/, SOCKET sockid_2 /*= INVALID_SOCKET*/) {
    if (!m_dtp) {
        switch (dtp_type) {
        case FTP_DTP_TYPE_PASSIVE_SEND:
            m_dtp = new ftp_dtp_passive_send(fpath, local_port, completion_port_2, sockid_2);
            break;
        case FTP_DTP_TYPE_PASSIVE_RECV:
            m_dtp = new ftp_dtp_passive_recv(fpath, local_port, completion_port_2, sockid_2);
            break;
        case FTP_DTP_TYPE_ACTIVE_SEND:
            m_dtp = new ftp_dtp_active_send(fpath, addr, port, completion_port_2, sockid_2);
            break;
        case FTP_DTP_TYPE_ACTIVE_RECV:
            m_dtp = new ftp_dtp_active_recv(fpath, addr, port, completion_port_2, sockid_2);
            break;
        default:
            assert(false);
        }
        m_dtp_type = dtp_type;

        return true;
    }

    return false;
}

bool ftp_dtp::ftp_dtp_2::abort() {
    switch (m_dtp_type) {
    case FTP_DTP_TYPE_PASSIVE_SEND:
        ((ftp_dtp_passive_send *)m_dtp)->abort();
        break;
    case FTP_DTP_TYPE_PASSIVE_RECV:
        ((ftp_dtp_passive_recv *)m_dtp)->abort();
        break;
    case FTP_DTP_TYPE_ACTIVE_SEND:
        ((ftp_dtp_active_send *)m_dtp)->abort();
        break;
    case FTP_DTP_TYPE_ACTIVE_RECV:
        ((ftp_dtp_active_recv *)m_dtp)->abort();
        break;
    default:
        assert(false);
    }

    return true;
}

bool ftp_dtp::ftp_dtp_2::close() {
    if (m_dtp) {
        this->abort();
        delete m_dtp;
        m_dtp = NULL;
        m_dtp_type = FTP_DTP_TYPE_NULL;
    }

    return true;
}

ftp_dtp::ftp_dtp_2::~ftp_dtp_2() {
}
