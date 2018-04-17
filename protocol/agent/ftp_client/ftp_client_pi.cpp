#include "ftp_client_pi.h"

ftp_client_pi::ftp_client_pi::ftp_client_pi() :network::Client() {
}

bool ftp_client_pi::ftp_client_pi::ftp_connect(const char *addr, unsigned int port, unsigned int *local_port/* = NULL*/) {
    if (connect(addr, port, local_port) != SOCKET_ERROR) {
        for (int i = 0; i < 10; ++i) {

            if (m_client_status == CLIENT_IO_STATUS::CIS_RSP_HANDLED) {
                printf("\n");

                return true;
            } else if (i == 9) {
                printf("\n");

                break;
            }
            printf(".");
            Sleep(500);
        }
    } else {
    }

    printf("Connect Error,Please Check Your Network.\n");

    return false;
}

bool ftp_client_pi::ftp_client_pi::ftp_send(const char *buffer, size_t count) {
    bool sending = false;
    while (true) {
        if (!sending && (m_client_status == CIS_RSP_HANDLED || m_client_status == CIS_CONNECTED)) {
            if (send(buffer, count) == false) {
                return false;
            }
            m_client_status = CIS_SENDING;
            sending = true;
        } else if (sending && m_client_status == CIS_RSP_HANDLED) {
            break;
        }
        Sleep(100);
    }

    return true;
}

ftp_client_pi::CLIENT_IO_STATUS ftp_client_pi::ftp_client_pi::get_io_status() {
    return m_client_status;
}

void ftp_client_pi::ftp_client_pi::on_connected(network::CLT_SOCKET_CONTEXT *sock_ctx) {
    m_client_status = CIS_CONNECTED;

    if (sock_ctx->m_bytes_transferred > 0) {
        m_client_inf.m_cmd_buffer.push(sock_ctx->m_szBuffer, sock_ctx->m_bytes_transferred);

        _handle_response();
    }
}

void ftp_client_pi::ftp_client_pi::on_sent(network::CLT_SOCKET_CONTEXT *sock_ctx) {
    m_client_status = CIS_SENT;
}

void ftp_client_pi::ftp_client_pi::on_recvd(network::CLT_SOCKET_CONTEXT *sock_ctx) {
    m_client_status = CIS_RECVD;

    m_client_inf.m_cmd_buffer.push(sock_ctx->m_szBuffer, sock_ctx->m_bytes_transferred);

    _handle_response();
}

void ftp_client_pi::ftp_client_pi::on_closed(network::CLT_SOCKET_CONTEXT *sock_ctx) {
    m_client_status = CIS_CLOSED;
    printf("OnClosed\n");
}

void ftp_client_pi::ftp_client_pi::_handle_response() {
    const char *str;

    while (str = m_client_inf.m_cmd_buffer.pop(), str) {
        printf("\t%s\n", str);
        fflush(stdout);

        m_client_status = CIS_RSP_HANDLED;
    }

    //TODO ERR CHECK
}
