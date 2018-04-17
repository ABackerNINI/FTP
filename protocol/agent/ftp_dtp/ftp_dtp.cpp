#include "ftp_dtp.h"

/*
 *  ftp_dtp_client
 */
bool ftp_dtp::ftp_dtp_client::abort() {
    close();
    m_freader.close();

    return true;
}

void ftp_dtp::ftp_dtp_client::set_ip(const char *ip) { m_ip = ip; }

void ftp_dtp::ftp_dtp_client::set_port(const char *port) { m_port = port; }

void ftp_dtp::ftp_dtp_client::set_fpath(const char *fpath) { m_fpath = fpath; }

size_t ftp_dtp::ftp_dtp_client::get_bytes_sent() { return m_bytes_sent; }

size_t ftp_dtp::ftp_dtp_client::get_fsize() { return m_fsize; }

void ftp_dtp::ftp_dtp_client::on_connected(network::CLT_SOCKET_CONTEXT *sock_ctx) {
    char buffer[DEFAULT_BUFFER_LEN];
    size_t count;

    if (m_freader.open(m_fpath) == NULL) {
        return;
    }
   
    m_fsize = m_freader.size();

    while (!m_freader.feof()) {
        count = m_freader.read(buffer, sizeof(char), DEFAULT_BUFFER_LEN);

        if (m_freader.ferror()) {
            break;
        }

        send(buffer, count);
        m_bytes_sent += count;
    }

    m_freader.close();

    close();
}

void ftp_dtp::ftp_dtp_client::on_sent(network::CLT_SOCKET_CONTEXT *sock_ctx) {
}

void ftp_dtp::ftp_dtp_client::on_recvd(network::CLT_SOCKET_CONTEXT *sock_ctx) {
}

void ftp_dtp::ftp_dtp_client::on_closed(network::CLT_SOCKET_CONTEXT *sock_ctx) {
}

/*
 *  ftp_dtp_server
 */
bool ftp_dtp::ftp_dtp_server::abort() {
    close();
    m_fwriter.close();

    return true;
}

void ftp_dtp::ftp_dtp_server::set_ip(const char *ip) { m_ip = ip; }

void ftp_dtp::ftp_dtp_server::set_port(const char *port) { m_port = port; }

void ftp_dtp::ftp_dtp_server::set_fpath(const char *fpath) { m_fpath = fpath; }

size_t ftp_dtp::ftp_dtp_server::get_bytes_recvd() { return m_bytes_recvd; }

void ftp_dtp::ftp_dtp_server::on_accepted(network::SVR_SOCKET_CONTEXT *sock_ctx) {
    m_fwriter.open(m_fpath);
}

void ftp_dtp::ftp_dtp_server::on_recvd(network::SVR_SOCKET_CONTEXT *sock_ctx) {
    m_fwriter.write(sock_ctx->m_buffer, sizeof(char), sock_ctx->m_bytes_transferred);
    m_bytes_recvd += sock_ctx->m_bytes_transferred;
}

void ftp_dtp::ftp_dtp_server::on_sent(network::SVR_SOCKET_CONTEXT *sock_ctx) {
}

void ftp_dtp::ftp_dtp_server::on_closed(network::SVR_SOCKET_CONTEXT *sock_ctx) {
    m_fwriter.close();
    printf("done\n");
}

/*
 *  ftp_dtp
 */
 //ftp_dtp::ftp_dtp::ftp_dtp() {
 //}
 //
 //bool ftp_dtp::ftp_dtp::start() {
 //    return false;
 //}
 //
 //bool ftp_dtp::ftp_dtp::abort() {
 //    return false;
 //}
 //
 //bool ftp_dtp::ftp_dtp::stop() {
 //    return false;
 //}
 //
 //ftp_dtp::ftp_dtp::~ftp_dtp() {
 //}
 //
 //enum ftp_dtp::STATUS ftp_dtp::ftp_dtp::get_status() {
 //    return STATUS(0);
 //}
 //
 //bool ftp_dtp::ftp_dtp::get_passive() {
 //    return false;
 //}
 //
 //enum ftp_dtp::STRUCTURE_TYPE ftp_dtp::ftp_dtp::get_structure_type() {
 //    return STRUCTURE_TYPE(0);
 //}
 //
 //enum ftp_dtp::DATA_TYPE ftp_dtp::ftp_dtp::get_data_type() {
 //    return DATA_TYPE(0);
 //}
 //
 //const char * ftp_dtp::ftp_dtp::get_ip() {
 //    return nullptr;
 //}
 //
 //int ftp_dtp::ftp_dtp::get_port() {
 //    return 0;
 //}
 //
 //void ftp_dtp::ftp_dtp::set_passive(bool passive) {
 //}
 //
 //void ftp_dtp::ftp_dtp::set_structure_type(STRUCTURE_TYPE sturcture_type) {
 //}
 //
 //void ftp_dtp::ftp_dtp::set_data_type(DATA_TYPE data_type) {
 //}
 //
 //void ftp_dtp::ftp_dtp::set_ip(const char *ip) {
 //}
 //
 //void ftp_dtp::ftp_dtp::set_port(int port) {
 //}
