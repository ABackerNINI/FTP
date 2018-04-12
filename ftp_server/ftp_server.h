#pragma once

#ifndef _NINI_FTP_SERVER_H_
#define _NINI_FTP_SERVER_H_

#include "../resource/common/common.h"
#include "../resource/ftp_cmds/ftp_cmds.h"
#include "../utility/network/network.h"
#include "../utility/string_buffer/string_buffer.h"

#define DEFAULT_BUFFER_LEN 1024
#define DEFAULT_USRNAME_BUFFER_LEN 100
#define DEFAULT_PASSWD_BUFFER_LEN 100
#define DEFAULT_DIR_BUFFER_LEN 100

#define _CMD(CMD)  network::pointer_cast<_CmdHandler>(&FtpServer::_CmdHandler##CMD)

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
    StringBuffer m_CmdBuffer;

    ClientInf() :
        m_IsPasv(false),
        m_Port(0),
        m_Status(CLS_CONNECTED) {
        m_Usrname[0] = '\0';
        m_Passwd[0] = '\0';
        m_Dir[0] = '/';
        m_Dir[1] = '\0';
    }
};

class FtpServer :public network::Server {
public:
    FtpServer();

protected:
    void OnAccepted(network::SVR_SOCKET_CONTEXT *_SocketContext) override;

    void OnRecvd(network::SVR_SOCKET_CONTEXT *_SocketContext) override;

    void OnSent(network::SVR_SOCKET_CONTEXT *_SocketContext) override;

    void OnClosed(network::SVR_SOCKET_CONTEXT *_SocketContext) override;

    enum FTP_CMDS CmdDispatch(char **_Str);

    bool _Handle(SOCKET _Socket, ClientInf *_ClientInf);

    bool _FtpSend(SOCKET _Socket, const char *_Buffer);

protected:

    //FtpServerClient				m_FtpServerClient;

protected:
    typedef void(*_CmdHandler)(FtpServer*, SOCKET, ClientInf*, char *);

    const _CmdHandler m_CmdHandler[FTP_CMDS_NUM + 1] = {//Need to Handle FTP_CMD_ERR
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

    void _CmdHandler_USER(SOCKET _Socket, ClientInf*, char *_Args);

    void _CmdHandler_PASS(SOCKET _Socket, ClientInf*, char *_Args);

    void _CmdHandler_CWD(SOCKET _Socket, ClientInf*, char *_Args);

    void _CmdHandler_PORT(SOCKET _Socket, ClientInf*, char *_Args);

    void _CmdHandler_PASV(SOCKET _Socket, ClientInf*, char *_Args);

    void _CmdHandler_RETR(SOCKET _Socket, ClientInf*, char *_Args);

    void _CmdHandler_STOR(SOCKET _Socket, ClientInf*, char *_Args);

    void _CmdHandler_DELE(SOCKET _Socket, ClientInf*, char *_Args);

    void _CmdHandler_RMD(SOCKET _Socket, ClientInf*, char *_Args);

    void _CmdHandler_MKD(SOCKET _Socket, ClientInf*, char *_Args);

    void _CmdHandler_PWD(SOCKET _Socket, ClientInf*, char *_Args);

    void _CmdHandler_LIST(SOCKET _Socket, ClientInf*, char *_Args);

    void _CmdHandler_HELP(SOCKET _Socket, ClientInf*, char *_Args);

    void _CmdHandler_NOOP(SOCKET _Socket, ClientInf*, char *_Args);

    void _CmdHandler_ERR(SOCKET _Socket, ClientInf*, char *_Args);

    void _CmdHandler_NOT_IMPLEMENTED(SOCKET _Socket, ClientInf*, char *_Args);
};

#endif //_NINI_FTP_SERVER_H_