#include "ftp_server_pi.h"
#include <assert.h>

/*-----------------------------------------------------------FtpServer Section-----------------------------------------------------------*/

ftp_server_pi::ftp_server_pi() :network::Server() {
    //network::ClientConfig _ClientConfig;
    //_ClientConfig.O0_WorkerThreads = 5;

    //m_FtpServerClient.SetConfig(_ClientConfig);
}

void ftp_server_pi::OnAccepted(network::SVR_SOCKET_CONTEXT * _SocketContext) {
    ClientInf *_ClientInf = new ClientInf();

    _ClientInf->m_Ip = _SocketContext->m_ClientAddr.sin_addr.S_un.S_addr;

    _FtpSend(_SocketContext->m_ClientSocket, "220 Welcome to NINI's FTP service.\r\n");

    if (_SocketContext->m_BytesTransferred > 0) {
        _ClientInf->m_CmdBuffer.push(_SocketContext->m_szBuffer, _SocketContext->m_BytesTransferred);

        _Handle(_SocketContext->m_ClientSocket, _ClientInf);
    }

    _SocketContext->m_Extra = _ClientInf;
}

void ftp_server_pi::OnRecvd(network::SVR_SOCKET_CONTEXT * _SocketContext) {
    ClientInf *_ClientInf = (ClientInf *)(_SocketContext->m_Extra);

    _ClientInf->m_CmdBuffer.push(_SocketContext->m_szBuffer, _SocketContext->m_BytesTransferred);

    _Handle(_SocketContext->m_ClientSocket, _ClientInf);
}

void ftp_server_pi::OnSent(network::SVR_SOCKET_CONTEXT * _SocketContext) {
}

void ftp_server_pi::OnClosed(network::SVR_SOCKET_CONTEXT * _SocketContext) {
    delete (ClientInf *)(_SocketContext->m_Extra);
}

bool ftp_server_pi::_Handle(SOCKET _Socket, ClientInf * _ClientInf) {
    char *_Str;
    char *_Args;
    int _Cmd;
    while (_Str = _ClientInf->m_CmdBuffer.pop(), _Str) {
        printf("%s\n", _Str);
        _Args = _Str;
        _Cmd = CmdDispatch(&_Args);

        if ((FTP_CMDS_INF[_Cmd].m_NeedArgs == FCNA_MANDATORY && _Args == NULL) || (FTP_CMDS_INF[_Cmd].m_NeedArgs == FCNA_NONE && _Args)) {
            _FtpSend(_Socket, "501 Syntax error in parameters or arguments.\r\n");
            continue;
        }

        //if ((_Cmd != FTP_CMD_USER&&_Cmd != FTP_CMD_PASS) && _ClientInf->m_Status == CLS_CONNECTED) {
        //	_FtpSend(_Socket, "530 Please login with USER and PASS.\r\n");
        //	continue;
        //}

        m_CmdHandler[_Cmd](this, _Socket, _ClientInf, _Args);
    }

    return false;
}

bool ftp_server_pi::_FtpSend(SOCKET _Socket, const char * _Buffer) {
    return Send(_Socket, _Buffer, strlen(_Buffer));
}

void ftp_server_pi::_CmdHandler_USER(SOCKET _Socket, ClientInf *_ClientInf, char * _Args) {
    switch (_ClientInf->m_Status) {
    case CLS_CONNECTED:
        _ClientInf->m_Status = CLS_USRNAME_SPECIFIED;
        memcpy(_ClientInf->m_Usrname, _Args, min(DEFAULT_USRNAME_BUFFER_LEN, strlen(_Args)));
        _FtpSend(_Socket, "331 User name ok,need password.\r\n");
        break;
    case CLS_USRNAME_SPECIFIED:
        memcpy(_ClientInf->m_Usrname, _Args, min(DEFAULT_USRNAME_BUFFER_LEN, strlen(_Args)));
        _FtpSend(_Socket, "331 User name ok,need password.\r\n");
        break;
    case CLS_PASSWORD_SPECIFIED:
        _ClientInf->m_Status = CLS_USRNAME_SPECIFIED;
        _FtpSend(_Socket, "331 User name ok,need password.\r\n");//TODO
        break;
    default:
        assert(false);
        break;
    }
}

void ftp_server_pi::_CmdHandler_PASS(SOCKET _Socket, ClientInf *_ClientInf, char * _Args) {
    switch (_ClientInf->m_Status) {
    case CLS_CONNECTED:
        _FtpSend(_Socket, "503 Need account for login.\r\n");
        break;
    case CLS_USRNAME_SPECIFIED:
        _ClientInf->m_Status = CLS_PASSWORD_SPECIFIED;
        memcpy(_ClientInf->m_Passwd, _Args, min(DEFAULT_PASSWD_BUFFER_LEN, strlen(_Args)));
        _FtpSend(_Socket, "230 User logged in, proceed.\r\n");
        break;
    case CLS_PASSWORD_SPECIFIED:
        _FtpSend(_Socket, "230 User logged in, proceed.\r\n");//TODO
        break;
    default:
        assert(false);
        break;
    }
}

void ftp_server_pi::_CmdHandler_CWD(SOCKET _Socket, ClientInf *, char * _Args) {
    _FtpSend(_Socket, "500 CWD.\r\n");
}

void ftp_server_pi::_CmdHandler_PORT(SOCKET _Socket, ClientInf *_ClientInf, char * _Args) {
    unsigned long _Port = std::atoi(_Args);
    if (_Port > 1023) {
        _FtpSend(_Socket, "200 Port command successful.\r\n");
    } else {
        _FtpSend(_Socket, "500 Port command faild.\r\n");
    }
}

void ftp_server_pi::_CmdHandler_PASV(SOCKET _Socket, ClientInf *, char * _Args) {
    _FtpSend(_Socket, "500 PASV.\r\n");
}

void ftp_server_pi::_CmdHandler_RETR(SOCKET _Socket, ClientInf *, char * _Args) {
    _FtpSend(_Socket, "500 RETR.\r\n");
}

void ftp_server_pi::_CmdHandler_STOR(SOCKET _Socket, ClientInf *, char * _Args) {
    _FtpSend(_Socket, "500 STOR.\r\n");
}

void ftp_server_pi::_CmdHandler_DELE(SOCKET _Socket, ClientInf *, char * _Args) {
}

void ftp_server_pi::_CmdHandler_RMD(SOCKET _Socket, ClientInf *, char * _Args) {
}

void ftp_server_pi::_CmdHandler_MKD(SOCKET _Socket, ClientInf *, char * _Args) {
}

void ftp_server_pi::_CmdHandler_PWD(SOCKET _Socket, ClientInf *, char * _Args) {
}

void ftp_server_pi::_CmdHandler_LIST(SOCKET _Socket, ClientInf *, char * _Args) {
}

void ftp_server_pi::_CmdHandler_HELP(SOCKET _Socket, ClientInf *, char * _Args) {
    if (_Args == NULL) {
        _FtpSend(_Socket, HELP_MSG);
    } else {
        char *_Tmp = _Args;
        FTP_CMDS _Cmd = CmdDispatch(&_Tmp);
        if (_Cmd != FTP_CMDS::FTP_CMD_ERR) {
            _FtpSend(_Socket, "214 ");
            _FtpSend(_Socket, FTP_CMDS_INF[_Cmd].m_HelpMsg);
            _FtpSend(_Socket, "\r\n");
        } else {
            _FtpSend(_Socket, "501 Unkown command \"");
            _FtpSend(_Socket, _Args);
            _FtpSend(_Socket, "\"\r\n");
        }
    }
}

void ftp_server_pi::_CmdHandler_NOOP(SOCKET _Socket, ClientInf *, char * _Args) {
    _FtpSend(_Socket, "200 ok\r\n");
}

void ftp_server_pi::_CmdHandler_ERR(SOCKET _Socket, ClientInf *, char * _Args) {
    _FtpSend(_Socket, "500 Syntax error, command unrecognized.\r\n");
}

void ftp_server_pi::_CmdHandler_NOT_IMPLEMENTED(SOCKET _Socket, ClientInf *, char * _Args) {
    _FtpSend(_Socket, "502 Command not implemented.\r\n");
}
