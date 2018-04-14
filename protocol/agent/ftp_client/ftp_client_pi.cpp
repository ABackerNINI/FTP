#include "ftp_client_pi.h"

ftp_client_pi::ftp_client_pi::ftp_client_pi() :network::Client() {
}

bool ftp_client_pi::ftp_client_pi::FtpConnect(const char *_Address, unsigned int _Port, unsigned int *_LocalPort/* = NULL*/) {
    //unsigned int _LocalPort = 0;
    if (Connect(_Address, _Port, _LocalPort) != SOCKET_ERROR) {
        for (int i = 0; i < 10; ++i) {

            if (m_ClientStatus == CLIENT_IO_STATUS::CIS_RSP_HANDLED) {
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

bool ftp_client_pi::ftp_client_pi::FtpSend(const char * _Buffer, size_t _Count) {
    bool _Sending = false;
    while (true) {
        if (!_Sending && (m_ClientStatus == CIS_RSP_HANDLED || m_ClientStatus == CIS_CONNECTED)) {
            if (Send(_Buffer, _Count) == false) {
                return false;
            }
            m_ClientStatus = CIS_SENDING;
            _Sending = true;
        } else if (_Sending && m_ClientStatus == CIS_RSP_HANDLED) {
            break;
        }
        Sleep(100);
    }

    return true;
}

ftp_client_pi::CLIENT_IO_STATUS ftp_client_pi::ftp_client_pi::GetIoStatus() {
    return m_ClientStatus;
}

bool ftp_client_pi::ftp_client_pi::Close() {
    return network::Client::Close();
}

void ftp_client_pi::ftp_client_pi::OnConnected(network::CLT_SOCKET_CONTEXT * _SocketContext) {
    m_ClientStatus = CIS_CONNECTED;

    if (_SocketContext->m_BytesTransferred > 0) {
        m_ClientInf.m_CmdBuffer.push(_SocketContext->m_szBuffer, _SocketContext->m_BytesTransferred);

        _HandleResponse();
    }
}

void ftp_client_pi::ftp_client_pi::OnSent(network::CLT_SOCKET_CONTEXT * _SocketContext) {
    m_ClientStatus = CIS_SENT;
}

void ftp_client_pi::ftp_client_pi::OnRecvd(network::CLT_SOCKET_CONTEXT * _SocketContext) {
    m_ClientStatus = CIS_RECVD;

    m_ClientInf.m_CmdBuffer.push(_SocketContext->m_szBuffer, _SocketContext->m_BytesTransferred);

    _HandleResponse();
}

void ftp_client_pi::ftp_client_pi::OnClosed(network::CLT_SOCKET_CONTEXT * _SocketContext) {
    m_ClientStatus = CIS_CLOSED;
    printf("OnClosed\n");
}

void ftp_client_pi::ftp_client_pi::_HandleResponse() {
    const char *_Str;

    while (_Str = m_ClientInf.m_CmdBuffer.pop(), _Str) {
        printf("\t%s\n", _Str);
        fflush(stdout);

        m_ClientStatus = CIS_RSP_HANDLED;
    }

    //TODO ERR CHECK
}
