#include "ftp_dtp.h"

/*
 *  ftp_dtp_client
 */
bool ftp_dtp::ftp_dtp_client::start() {
    return false;
}

bool ftp_dtp::ftp_dtp_client::abort() {
    return false;
}

bool ftp_dtp::ftp_dtp_client::stop() {
    return false;
}

void ftp_dtp::ftp_dtp_client::OnConnected(network::CLT_SOCKET_CONTEXT *sock_ctx) {
    m_fr.open("1.txt");

    char buffer[1024];
    size_t count;

    while (!m_fr.feof()) {
        count = m_fr.read(buffer, sizeof(char), 1024);

        if (m_fr.ferror()) {
            break;
        }

        Send(buffer, count);
    }

    m_fr.close();
}

void ftp_dtp::ftp_dtp_client::OnSent(network::CLT_SOCKET_CONTEXT *sock_ctx) {
}

void ftp_dtp::ftp_dtp_client::OnRecvd(network::CLT_SOCKET_CONTEXT *sock_ctx) {
}

void ftp_dtp::ftp_dtp_client::OnClosed(network::CLT_SOCKET_CONTEXT *sock_ctx) {
}

/*
 *  ftp_dtp_server
 */
bool ftp_dtp::ftp_dtp_server::start() {
    return false;
}

bool ftp_dtp::ftp_dtp_server::abort() {
    return false;
}

bool ftp_dtp::ftp_dtp_server::stop() {
    return false;
}

void ftp_dtp::ftp_dtp_server::OnAccepted(network::SVR_SOCKET_CONTEXT *sock_ctx) {
    m_fw.open("test");
}

void ftp_dtp::ftp_dtp_server::OnRecvd(network::SVR_SOCKET_CONTEXT *sock_ctx) {
    m_fw.write(sock_ctx->m_buffer, sizeof(char), sock_ctx->m_bytes_transferred);
}

void ftp_dtp::ftp_dtp_server::OnSent(network::SVR_SOCKET_CONTEXT *sock_ctx) {
}

void ftp_dtp::ftp_dtp_server::OnClosed(network::SVR_SOCKET_CONTEXT *sock_ctx) {
    m_fw.close();
    printf("done\n");
}

/*
 *  ftp_dtp
 */
ftp_dtp::ftp_dtp::ftp_dtp() {
}

bool ftp_dtp::ftp_dtp::start() {
    return false;
}

bool ftp_dtp::ftp_dtp::abort() {
    return false;
}

bool ftp_dtp::ftp_dtp::stop() {
    return false;
}

ftp_dtp::ftp_dtp::~ftp_dtp() {
}

enum ftp_dtp::STATUS ftp_dtp::ftp_dtp::get_status() {
    return STATUS(0);
}

bool ftp_dtp::ftp_dtp::get_passive() {
    return false;
}

enum ftp_dtp::STRUCTURE_TYPE ftp_dtp::ftp_dtp::get_structure_type() {
    return STRUCTURE_TYPE(0);
}

enum ftp_dtp::DATA_TYPE ftp_dtp::ftp_dtp::get_data_type() {
    return DATA_TYPE(0);
}

const char * ftp_dtp::ftp_dtp::get_ip() {
    return nullptr;
}

int ftp_dtp::ftp_dtp::get_port() {
    return 0;
}

void ftp_dtp::ftp_dtp::set_passive(bool passive) {
}

void ftp_dtp::ftp_dtp::set_structure_type(STRUCTURE_TYPE sturcture_type) {
}

void ftp_dtp::ftp_dtp::set_data_type(DATA_TYPE data_type) {
}

void ftp_dtp::ftp_dtp::set_ip(const char *ip) {
}

void ftp_dtp::ftp_dtp::set_port(int port) {
}
