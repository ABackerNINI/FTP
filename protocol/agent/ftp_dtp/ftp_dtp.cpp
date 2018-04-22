#include "ftp_dtp.h"

/*
 *  ftp_dtp_client
 */
ftp_dtp::ftp_dtp_client::ftp_dtp_client() {
}

bool ftp_dtp::ftp_dtp_client::abort() {
    close();

    return true;
}

void ftp_dtp::ftp_dtp_client::set_fpath(const char *fpath) { m_fpath = fpath; }

size_t ftp_dtp::ftp_dtp_client::get_bytes_sent() { return m_bytes_sent; }

size_t ftp_dtp::ftp_dtp_client::get_fsize() { return m_fsize; }

void ftp_dtp::ftp_dtp_client::event_handler(network::CLT_SOCKET_CONTEXT *sock_ctx, int ev) {
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

void ftp_dtp::ftp_dtp_client::_on_connected(network::CLT_SOCKET_CONTEXT *sock_ctx) {
    char buffer[DEFAULT_BUFFER_LEN];
    size_t count;

    if (m_freader.open(m_fpath) == NULL) {
        return;
    }

    m_fsize = m_freader.size();

    while (!m_freader.feof()) {
        count = m_freader.read(buffer, sizeof(char), DEFAULT_BUFFER_LEN);

        if (m_freader.ferror() || !send(buffer, count)) {
            break;
        }
        m_bytes_sent += count;
    }

    m_freader.close();

    close_connection();

    notify_worker_threads_to_exit();
}

void ftp_dtp::ftp_dtp_client::_on_sent(network::CLT_SOCKET_CONTEXT *sock_ctx) {
}

void ftp_dtp::ftp_dtp_client::_on_recvd(network::CLT_SOCKET_CONTEXT *sock_ctx) {
}

void ftp_dtp::ftp_dtp_client::_on_closed(network::CLT_SOCKET_CONTEXT *sock_ctx) {
}

/*
 *  ftp_dtp_server
 */
ftp_dtp::ftp_dtp_server::ftp_dtp_server() {
}

bool ftp_dtp::ftp_dtp_server::abort() {
    close();

    return true;
}

void ftp_dtp::ftp_dtp_server::set_fpath(const char *fpath) { m_fpath = fpath; }

size_t ftp_dtp::ftp_dtp_server::get_bytes_recvd() { return m_bytes_recvd; }

void ftp_dtp::ftp_dtp_server::event_handler(network::SVR_SOCKET_CONTEXT *sock_ctx, int ev) {
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

void ftp_dtp::ftp_dtp_server::_on_accepted(network::SVR_SOCKET_CONTEXT *sock_ctx) {
    m_fwriter.open(m_fpath);
}

void ftp_dtp::ftp_dtp_server::_on_recvd(network::SVR_SOCKET_CONTEXT *sock_ctx) {
    m_fwriter.write(sock_ctx->m_buffer, sizeof(char), sock_ctx->m_bytes_transferred);
    m_bytes_recvd += sock_ctx->m_bytes_transferred;
}

void ftp_dtp::ftp_dtp_server::_on_sent(network::SVR_SOCKET_CONTEXT *sock_ctx) {
}

void ftp_dtp::ftp_dtp_server::_on_closed(network::SVR_SOCKET_CONTEXT *sock_ctx) {
    m_fwriter.close();
    close_connection(sock_ctx->m_client_sockid);
    close_listen();
    notify_worker_threads_to_exit();
    printf("done\n");
}

/*
 *  ftp_dtp
 */
ftp_dtp::ftp_dtp::ftp_dtp() :m_passive(false), m_client(NULL), m_server(NULL) {
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
        m_server = new ftp_dtp_server();
        m_server->set_config(*server_config);
        m_server->set_fpath(m_fpath);
        m_server->start_listen(m_port);
    } else {
        m_client = new ftp_dtp_client();
        m_client->set_fpath(m_fpath);
        m_client->connect(m_addr, m_port);
    }

    return true;
}

bool ftp_dtp::ftp_dtp::abort() {
    return close();
}

bool ftp_dtp::ftp_dtp::close() {
    if (m_server) {
        m_server->close();
        delete m_server;
        m_server = NULL;
    }
    if (m_client) {
        m_client->close();
        delete m_client;
        m_client = NULL;
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

void ftp_dtp::ftp_dtp::set_fpath(const char *fpath) {
    m_fpath = fpath;
}

void ftp_dtp::ftp_dtp::set_addr(const char *addr) {
    m_addr = addr;
}

void ftp_dtp::ftp_dtp::set_port(unsigned int port) {
    m_port = port;
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