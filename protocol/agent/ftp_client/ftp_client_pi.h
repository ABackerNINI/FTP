#pragma once

#ifndef _NINI_FTP_CLIENT_PI_H_
#define _NINI_FTP_CLIENT_PI_H_

#include "../ftp_dtp/ftp_dtp.h"
#include "../../resource/ftp_cmds/ftp_cmds.h"
#include "../../../utility/network/network.h"
#include "../../../utility/string_buffer/string_buffer.h"

namespace ftp_client_pi {

#define DEFAULT_BUFFER_LEN 1024

    enum CLIENT_IO_STATUS {
        CIS_CONNECTING,
        CIS_CONNECTED,
        CIS_SENDING,
        CIS_SENT,
        CIS_RECVD,
        CIS_RSP_HANDLED,
        CIS_CLOSED
    };

    enum CLIENT_LOGIN_STATUS {
        CLS_NOT_CONNECTED,
        CLS_CONNECTED,
        CLS_USERNAME_SPECIFIED,
        CLS_PASSWORD_SPECIFIED,
        CLS_LOGIN_SUCCESS,
        CLS_DISCONNECTED
    };

    struct ClientInf {
        string_buffer m_cmd_buffer;
    };

    class ftp_client_pi :public network::Client {
    public:
        ftp_client_pi();

        bool ftp_connect(const char *addr, unsigned int port, unsigned int *local_port = NULL);

        bool ftp_send(const char *buffer, size_t count);

        CLIENT_IO_STATUS get_io_status();

    protected:
        void on_connected(network::CLT_SOCKET_CONTEXT *sock_ctx) override;

        void on_sent(network::CLT_SOCKET_CONTEXT *sock_ctx)override;

        void on_recvd(network::CLT_SOCKET_CONTEXT *sock_ctx)override;

        void on_closed(network::CLT_SOCKET_CONTEXT *sock_ctx)override;

        void _handle_response();

    protected:
        ftp_cmds::FTP_CMDS	m_last_cmd;
        ClientInf			m_client_inf;
        CLIENT_IO_STATUS	m_client_status;
    };
}

#endif //_NINI_FTP_CLIENT_PI_H_