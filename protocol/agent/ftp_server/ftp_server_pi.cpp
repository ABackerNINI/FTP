#include "ftp_server_pi.h"
#include <assert.h>

ftp_server_pi::ClientInf::ClientInf() :
    m_IsPasv(false),
    m_Port(0),
    m_Status(CLS_CONNECTED) {
    m_Usrname[0] = '\0';
    m_Passwd[0] = '\0';
    m_Dir[0] = '/';
    m_Dir[1] = '\0';
}

/*-----------------------------------------------------------FtpServer Section-----------------------------------------------------------*/

ftp_server_pi::ftp_server_pi::ftp_server_pi() :network::Server() {
}

void ftp_server_pi::ftp_server_pi::OnAccepted(network::SVR_SOCKET_CONTEXT *sock_ctx) {
    ClientInf *_ClientInf = new ClientInf();

    _ClientInf->m_Ip = sock_ctx->m_client_addr.sin_addr.S_un.S_addr;

    _FtpSend(sock_ctx->m_client_sockid, "220 Welcome to NINI's FTP service.\r\n");

    if (sock_ctx->m_bytes_transferred > 0) {
        _ClientInf->m_CmdBuffer.push(sock_ctx->m_buffer, sock_ctx->m_bytes_transferred);

        _Handle(sock_ctx->m_client_sockid, _ClientInf);
    }

    sock_ctx->m_extra = _ClientInf;
}

void ftp_server_pi::ftp_server_pi::OnRecvd(network::SVR_SOCKET_CONTEXT *sock_ctx) {
    ClientInf *_ClientInf = (ClientInf *)(sock_ctx->m_extra);

    _ClientInf->m_CmdBuffer.push(sock_ctx->m_buffer, sock_ctx->m_bytes_transferred);

    _Handle(sock_ctx->m_client_sockid, _ClientInf);
}

void ftp_server_pi::ftp_server_pi::OnSent(network::SVR_SOCKET_CONTEXT *sock_ctx) {
}

void ftp_server_pi::ftp_server_pi::OnClosed(network::SVR_SOCKET_CONTEXT *sock_ctx) {
    delete (ClientInf *)(sock_ctx->m_extra);
}

bool ftp_server_pi::ftp_server_pi::_Handle(SOCKET _Sockid, ClientInf *_ClientInf) {
    char *_Str;
    char *_Args;
    int _Cmd;
    while (_Str = _ClientInf->m_CmdBuffer.pop(), _Str) {
        printf("%s\n", _Str);
        _Args = _Str;
        _Cmd = ftp_cmds::CmdDispatch(&_Args);

        if ((ftp_cmds::FTP_CMDS_INF[_Cmd].m_NeedArgs == ftp_cmds::FCNA_MANDATORY && _Args == NULL) || (ftp_cmds::FTP_CMDS_INF[_Cmd].m_NeedArgs == ftp_cmds::FCNA_NONE && _Args)) {
            _FtpSend(_Sockid, "501 Syntax error in parameters or arguments.\r\n");
            continue;
        }

        //if ((_Cmd != FTP_CMD_USER&&_Cmd != FTP_CMD_PASS) && _ClientInf->m_Status == CLS_CONNECTED) {
        //	_FtpSend(_Socket, "530 Please login with USER and PASS.\r\n");
        //	continue;
        //}

        m_CmdHandler[_Cmd](this, _Sockid, _ClientInf, _Args);
    }

    return false;
}

bool ftp_server_pi::ftp_server_pi::_FtpSend(SOCKET _Sockid, const char *_Buffer) {
    return Send(_Sockid, _Buffer, strlen(_Buffer));
}

void ftp_server_pi::ftp_server_pi::_CmdHandler_USER(SOCKET _Sockid, ClientInf *_ClientInf, char *_Args) {
    switch (_ClientInf->m_Status) {
    case CLS_CONNECTED:
        _ClientInf->m_Status = CLS_USRNAME_SPECIFIED;
        memcpy(_ClientInf->m_Usrname, _Args, min(DEFAULT_USRNAME_BUFFER_LEN, strlen(_Args)));
        _FtpSend(_Sockid, "331 User name ok,need password.\r\n");
        break;
    case CLS_USRNAME_SPECIFIED:
        memcpy(_ClientInf->m_Usrname, _Args, min(DEFAULT_USRNAME_BUFFER_LEN, strlen(_Args)));
        _FtpSend(_Sockid, "331 User name ok,need password.\r\n");
        break;
    case CLS_PASSWORD_SPECIFIED:
        _ClientInf->m_Status = CLS_USRNAME_SPECIFIED;
        _FtpSend(_Sockid, "331 User name ok,need password.\r\n");//TODO
        break;
    default:
        assert(false);
        break;
    }
}

void ftp_server_pi::ftp_server_pi::_CmdHandler_PASS(SOCKET _Sockid, ClientInf *_ClientInf, char *_Args) {
    switch (_ClientInf->m_Status) {
    case CLS_CONNECTED:
        _FtpSend(_Sockid, "503 Need account for login.\r\n");
        break;
    case CLS_USRNAME_SPECIFIED:
        _ClientInf->m_Status = CLS_PASSWORD_SPECIFIED;
        memcpy(_ClientInf->m_Passwd, _Args, min(DEFAULT_PASSWD_BUFFER_LEN, strlen(_Args)));
        _FtpSend(_Sockid, "230 User logged in, proceed.\r\n");
        break;
    case CLS_PASSWORD_SPECIFIED:
        _FtpSend(_Sockid, "230 User logged in, proceed.\r\n");//TODO
        break;
    default:
        assert(false);
        break;
    }
}

void ftp_server_pi::ftp_server_pi::_CmdHandler_CWD(SOCKET _Sockid, ClientInf *_ClientInf, char *_Args) {
    _FtpSend(_Sockid, "500 CWD.\r\n");
}

void ftp_server_pi::ftp_server_pi::_CmdHandler_PORT(SOCKET _Sockid, ClientInf *_ClientInf, char *_Args) {
    unsigned long _Port = std::atoi(_Args);
    if (_Port > 1023) {
        _FtpSend(_Sockid, "200 Port command successful.\r\n");
    } else {
        _FtpSend(_Sockid, "500 Port command faild.\r\n");
    }
}

void ftp_server_pi::ftp_server_pi::_CmdHandler_PASV(SOCKET _Sockid, ClientInf *_ClientInf, char *_Args) {
    _FtpSend(_Sockid, "500 PASV.\r\n");
}

void ftp_server_pi::ftp_server_pi::_CmdHandler_RETR(SOCKET _Sockid, ClientInf *_ClientInf, char *_Args) {
    _FtpSend(_Sockid, "500 RETR.\r\n");
}

void ftp_server_pi::ftp_server_pi::_CmdHandler_STOR(SOCKET _Sockid, ClientInf *_ClientInf, char *_Args) {
    _FtpSend(_Sockid, "500 STOR.\r\n");
}

void ftp_server_pi::ftp_server_pi::_CmdHandler_DELE(SOCKET _Sockid, ClientInf *_ClientInf, char *_Args) {
}

void ftp_server_pi::ftp_server_pi::_CmdHandler_RMD(SOCKET _Sockid, ClientInf *_ClientInf, char *_Args) {
}

void ftp_server_pi::ftp_server_pi::_CmdHandler_MKD(SOCKET _Sockid, ClientInf *_ClientInf, char *_Args) {
}

void ftp_server_pi::ftp_server_pi::_CmdHandler_PWD(SOCKET _Sockid, ClientInf *_ClientInf, char *_Args) {
}

void ftp_server_pi::ftp_server_pi::_CmdHandler_LIST(SOCKET _Sockid, ClientInf *_ClientInf, char *_Args) {
}

void ftp_server_pi::ftp_server_pi::_CmdHandler_HELP(SOCKET _Sockid, ClientInf *_ClientInf, char *_Args) {
    if (_Args == NULL) {
        _FtpSend(_Sockid, ftp_cmds::HELP_MSG);
    } else {
        char *_Tmp = _Args;
        ftp_cmds::FTP_CMDS _Cmd = ftp_cmds::CmdDispatch(&_Tmp);
        if (_Cmd != ftp_cmds::FTP_CMDS::FTP_CMD_ERR) {
            _FtpSend(_Sockid, "214 ");
            _FtpSend(_Sockid, ftp_cmds::FTP_CMDS_INF[_Cmd].m_HelpMsg);
            _FtpSend(_Sockid, "\r\n");
        } else {
            _FtpSend(_Sockid, "501 Unkown command \"");
            _FtpSend(_Sockid, _Args);
            _FtpSend(_Sockid, "\"\r\n");
        }
    }
}

void ftp_server_pi::ftp_server_pi::_CmdHandler_NOOP(SOCKET _Sockid, ClientInf *_ClientInf, char *_Args) {
    _FtpSend(_Sockid, "200 ok\r\n");
}

void ftp_server_pi::ftp_server_pi::_CmdHandler_ERR(SOCKET _Sockid, ClientInf *_ClientInf, char *_Args) {
    _FtpSend(_Sockid, "500 Syntax error, command unrecognized.\r\n");
}

void ftp_server_pi::ftp_server_pi::_CmdHandler_NOT_IMPLEMENTED(SOCKET _Sockid, ClientInf *_ClientInf, char *_Args) {
    _FtpSend(_Sockid, "502 Command not implemented.\r\n");
}
