#pragma once

#ifndef _NINI_FTP_CLIENT_PI_H_
#define _NINI_FTP_CLIENT_PI_H_

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
        string_buffer m_CmdBuffer;
    };

    class ftp_client_pi :public network::Client {
    public:
        ftp_client_pi();

        bool FtpConnect(const char *_Address, unsigned int _Port, unsigned int *_LocalPort = NULL);

        bool FtpSend(const char *_Buffer, size_t _Count);

        CLIENT_IO_STATUS GetIoStatus();

        bool Close();

    protected:
        void OnConnected(network::CLT_SOCKET_CONTEXT *sock_ctx) override;

        void OnSent(network::CLT_SOCKET_CONTEXT *sock_ctx)override;

        void OnRecvd(network::CLT_SOCKET_CONTEXT *sock_ctx)override;

        void OnClosed(network::CLT_SOCKET_CONTEXT *sock_ctx)override;

        void _HandleResponse();

    protected:
        ftp_cmds::FTP_CMDS	m_LastCmd;
        ClientInf			m_ClientInf;
        CLIENT_IO_STATUS	m_ClientStatus;
    };
}

#endif //_NINI_FTP_CLIENT_PI_H_