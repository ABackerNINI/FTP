#pragma once

#ifndef _NINI_FTP_SERVER_PI_H_
#define _NINI_FTP_SERVER_PI_H_

#include "../../resource/ftp_cmds/ftp_cmds.h"
#include "../../../common/common.h"
#include "../../../utility/network/network.h"
#include "../../../utility/string_buffer/string_buffer.h"

namespace ftp_server_pi {

#define DEFAULT_BUFFER_LEN 1024
#define DEFAULT_USRNAME_BUFFER_LEN 100
#define DEFAULT_PASSWD_BUFFER_LEN 100
#define DEFAULT_DIR_BUFFER_LEN 100

#define _CMD(CMD)  network::pointer_cast<_CmdHandler>(&ftp_server_pi::_CmdHandler##CMD)

    enum CLIENT_LOGIN_STATUS {
        CLS_CONNECTED,
        CLS_USRNAME_SPECIFIED,
        CLS_PASSWORD_SPECIFIED //Login Successfully
    };

    struct ClientInf {
        char	m_Usrname[100];
        char	m_Passwd[100];
        char	m_Dir[100];
        bool	m_IsPasv;
        int		m_Port;
        unsigned long  m_Ip;
        CLIENT_LOGIN_STATUS m_Status;
        string_buffer m_CmdBuffer;

        ClientInf();
    };

    class ftp_server_pi :public network::Server {
    public:
        ftp_server_pi();

    protected:
        void OnAccepted(network::SVR_SOCKET_CONTEXT *_SocketContext) override;

        void OnRecvd(network::SVR_SOCKET_CONTEXT *_SocketContext) override;

        void OnSent(network::SVR_SOCKET_CONTEXT *_SocketContext) override;

        void OnClosed(network::SVR_SOCKET_CONTEXT *_SocketContext) override;

        bool _Handle(SOCKET _Sockid, ClientInf *_ClientInf);

        bool _FtpSend(SOCKET _Sockid, const char *_Buffer);

    protected:

        //FtpServerClient				m_FtpServerClient;

    protected:
        typedef void(*_CmdHandler)(ftp_server_pi*, SOCKET, ClientInf*, char *);

        const _CmdHandler m_CmdHandler[ftp_cmds::FTP_CMDS_NUM + 1] = {//Need to Handle FTP_CMD_ERR
            { _CMD(_USER)},
            { _CMD(_PASS)},
            { _CMD(_NOT_IMPLEMENTED)},//ACCT
            { _CMD(_CWD)},
            { _CMD(_NOT_IMPLEMENTED)},//CDUP
            { _CMD(_NOT_IMPLEMENTED)},//SMNT
            { _CMD(_NOT_IMPLEMENTED)},//QUIT
            { _CMD(_NOT_IMPLEMENTED)},//REIN
            { _CMD(_PORT)},//PORT
            { _CMD(_PASV)},
            { _CMD(_NOT_IMPLEMENTED)},//TYPE
            { _CMD(_NOT_IMPLEMENTED)},//STRU
            { _CMD(_NOT_IMPLEMENTED)},//MODE
            { _CMD(_RETR)},
            { _CMD(_STOR)},
            { _CMD(_NOT_IMPLEMENTED)},//STOU
            { _CMD(_NOT_IMPLEMENTED)},//APPE
            { _CMD(_NOT_IMPLEMENTED)},//ALLO
            { _CMD(_NOT_IMPLEMENTED)},//REST
            { _CMD(_NOT_IMPLEMENTED)},//RNFR
            { _CMD(_NOT_IMPLEMENTED)},//RNTO
            { _CMD(_NOT_IMPLEMENTED)},//ABOR
            { _CMD(_NOT_IMPLEMENTED)},//DELE
            { _CMD(_NOT_IMPLEMENTED)},//RMD
            { _CMD(_NOT_IMPLEMENTED)},//MKD
            { _CMD(_NOT_IMPLEMENTED)},//PWD
            { _CMD(_NOT_IMPLEMENTED)},//LIST
            { _CMD(_NOT_IMPLEMENTED)},//NLST
            { _CMD(_NOT_IMPLEMENTED)},//SITE
            { _CMD(_NOT_IMPLEMENTED)},//SYST
            { _CMD(_NOT_IMPLEMENTED)},//STAT
            { _CMD(_HELP)},
            { _CMD(_NOOP)},
            { _CMD(_ERR)}
        };

        void _CmdHandler_USER(SOCKET _Sockid, ClientInf*, char *_Args);

        void _CmdHandler_PASS(SOCKET _Sockid, ClientInf*, char *_Args);

        void _CmdHandler_CWD(SOCKET _Sockid, ClientInf*, char *_Args);

        void _CmdHandler_PORT(SOCKET _Sockid, ClientInf*, char *_Args);

        void _CmdHandler_PASV(SOCKET _Sockid, ClientInf*, char *_Args);

        void _CmdHandler_RETR(SOCKET _Sockid, ClientInf*, char *_Args);

        void _CmdHandler_STOR(SOCKET _Sockid, ClientInf*, char *_Args);

        void _CmdHandler_DELE(SOCKET _Sockid, ClientInf*, char *_Args);

        void _CmdHandler_RMD(SOCKET _Sockid, ClientInf*, char *_Args);

        void _CmdHandler_MKD(SOCKET _Sockid, ClientInf*, char *_Args);

        void _CmdHandler_PWD(SOCKET _Sockid, ClientInf*, char *_Args);

        void _CmdHandler_LIST(SOCKET _Sockid, ClientInf*, char *_Args);

        void _CmdHandler_HELP(SOCKET _Sockid, ClientInf*, char *_Args);

        void _CmdHandler_NOOP(SOCKET _Sockid, ClientInf*, char *_Args);

        void _CmdHandler_ERR(SOCKET _Sockid, ClientInf*, char *_Args);

        void _CmdHandler_NOT_IMPLEMENTED(SOCKET _Sockid, ClientInf*, char *_Args);
    };
}

#endif //_NINI_FTP_SERVER_PI_H_