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

bool ftp_client_pi::ftp_client_pi::ftp_input(char *buffer, size_t count) {
    _handle_input(buffer, count);

    _ftp_send(buffer, count);

    return true;
}

ftp_client_pi::CLIENT_IO_STATUS ftp_client_pi::ftp_client_pi::get_io_status() {
    return m_client_status;
}

bool ftp_client_pi::ftp_client_pi::_ftp_send(char *buffer, size_t count) {
    buffer[count - 1] = '\r';
    buffer[count++] = '\n';
    buffer[count++] = '\0';//mark:?

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

bool ftp_client_pi::ftp_client_pi::_handle_input(char *buffer, size_t count) {
    char *args = buffer;
    int cmd_id = ftp_cmds::cmd_dispatch(&args);

    if ((ftp_cmds::FTP_CMDS_INF[cmd_id].m_need_args == ftp_cmds::FCNA_MANDATORY && args == NULL) || (ftp_cmds::FTP_CMDS_INF[cmd_id].m_need_args == ftp_cmds::FCNA_NONE && args)) {
        return false;
    }

    m_cmd_handler[cmd_id](this, args);

    return false;
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

void ftp_client_pi::ftp_client_pi::on_connected(network::CLT_SOCKET_CONTEXT *sock_ctx) {
    m_client_status = CIS_CONNECTED;

    if (sock_ctx->m_bytes_transferred > 0) {
        m_client_inf.m_cmd_buffer.push(sock_ctx->m_buffer, sock_ctx->m_bytes_transferred);

        _handle_response();
    }
}

void ftp_client_pi::ftp_client_pi::on_sent(network::CLT_SOCKET_CONTEXT *sock_ctx) {
    m_client_status = CIS_SENT;
}

void ftp_client_pi::ftp_client_pi::on_recvd(network::CLT_SOCKET_CONTEXT *sock_ctx) {
    m_client_status = CIS_RECVD;

    m_client_inf.m_cmd_buffer.push(sock_ctx->m_buffer, sock_ctx->m_bytes_transferred);

    _handle_response();
}

void ftp_client_pi::ftp_client_pi::on_closed(network::CLT_SOCKET_CONTEXT *sock_ctx) {
    m_client_status = CIS_CLOSED;
    printf("OnClosed\n");
}

void ftp_client_pi::ftp_client_pi::cmd_handler_USER(char *args) {
}

void ftp_client_pi::ftp_client_pi::cmd_handler_PASS(char *args) {
}

void ftp_client_pi::ftp_client_pi::cmd_handler_CWD(char *args) {
}

void ftp_client_pi::ftp_client_pi::cmd_handler_PORT(char *args) {
}

void ftp_client_pi::ftp_client_pi::cmd_handler_PASV(char *args) {
}

void ftp_client_pi::ftp_client_pi::cmd_handler_RETR(char *args) {
}

void ftp_client_pi::ftp_client_pi::cmd_handler_STOR(char *args) {
    m_dtp.set_addr("192.168.1.107");
    m_dtp.set_port(20);
    m_dtp.set_fpath("file_to_be_sent");
    m_dtp.start();
}

void ftp_client_pi::ftp_client_pi::cmd_handler_DELE(char *args) {
}

void ftp_client_pi::ftp_client_pi::cmd_handler_RMD(char *args) {
}

void ftp_client_pi::ftp_client_pi::cmd_handler_MKD(char *args) {
}

void ftp_client_pi::ftp_client_pi::cmd_handler_PWD(char *args) {
}

void ftp_client_pi::ftp_client_pi::cmd_handler_LIST(char *args) {
}

void ftp_client_pi::ftp_client_pi::cmd_handler_HELP(char *args) {
}

void ftp_client_pi::ftp_client_pi::cmd_handler_NOOP(char *args) {
}

void ftp_client_pi::ftp_client_pi::cmd_handler_ERR(char *args) {
}

void ftp_client_pi::ftp_client_pi::cmd_handler_NOT_IMPLEMENTED(char *args) {
}
