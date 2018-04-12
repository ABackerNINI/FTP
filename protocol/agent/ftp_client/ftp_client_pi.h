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

    struct FtpClientConfig :network::ClientConfig {
        int M_Port;
    };

    struct ClientInf {
        string_buffer m_CmdBuffer;
    };

    class ftp_client_pi :public network::Client {
    public:
        ftp_client_pi();

        ftp_client_pi(const FtpClientConfig &_FtpClientConfig);

        void SetConfig(const FtpClientConfig &_FtpClientConfig);

        bool FtpConnect(const network::IP_PORT *_IpPort);

        bool FtpSend(const char *_Buffer, size_t _Count);

        CLIENT_IO_STATUS GetIoStatus();

        bool Close();

    protected:
        void OnConnected(network::CLT_SOCKET_CONTEXT *_SocketContext) override;

        void OnSent(network::CLT_SOCKET_CONTEXT *_SocketContext)override;

        void OnRecvd(network::CLT_SOCKET_CONTEXT *_SocketContext)override;

        void OnClosed(network::CLT_SOCKET_CONTEXT *_SocketContext)override;

        void _HandleResponse();

    protected:
        SOCKET				m_Socket;

        ftp_cmds::FTP_CMDS			m_LastCmd;

        ClientInf			m_ClientInf;

        CLIENT_IO_STATUS	m_ClientStatus;

        //FtpClientData		m_FtpClientData;

        //FtpClientServer		m_FtpClientServer;
    };
}

#endif //_NINI_FTP_CLIENT_PI_H_