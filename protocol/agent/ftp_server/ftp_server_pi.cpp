#include "ftp_server_pi.h"
#include <assert.h>

ftp_server_pi::ClientInf::ClientInf() :
    m_is_pasv(false),
    m_port(0),
    m_status(CLS_CONNECTED) {
    m_user_name[0] = '\0';
    m_passwd[0] = '\0';
    m_working_dir[0] = '/';
    m_working_dir[1] = '\0';
}

/*-----------------------------------------------------------FtpServer Section-----------------------------------------------------------*/

ftp_server_pi::ftp_server_pi::ftp_server_pi() :network::Server() {
}

void ftp_server_pi::ftp_server_pi::on_accepted(network::SVR_SOCKET_CONTEXT *sock_ctx) {
    ClientInf *client_inf = new ClientInf();

    client_inf->m_ip = sock_ctx->m_client_addr.sin_addr.S_un.S_addr;

    _ftp_send(sock_ctx->m_client_sockid, "220 Welcome to NINI's FTP service.\r\n");

    if (sock_ctx->m_bytes_transferred > 0) {
        client_inf->m_cmd_buffer.push(sock_ctx->m_buffer, sock_ctx->m_bytes_transferred);

        _handle(sock_ctx->m_client_sockid, client_inf);
    }

    sock_ctx->m_extra = client_inf;
}

void ftp_server_pi::ftp_server_pi::on_recvd(network::SVR_SOCKET_CONTEXT *sock_ctx) {
    ClientInf *client_inf = (ClientInf *)(sock_ctx->m_extra);

    client_inf->m_cmd_buffer.push(sock_ctx->m_buffer, sock_ctx->m_bytes_transferred);

    _handle(sock_ctx->m_client_sockid, client_inf);
}

void ftp_server_pi::ftp_server_pi::on_sent(network::SVR_SOCKET_CONTEXT *sock_ctx) {
}

void ftp_server_pi::ftp_server_pi::on_closed(network::SVR_SOCKET_CONTEXT *sock_ctx) {
    delete (ClientInf *)(sock_ctx->m_extra);
}

bool ftp_server_pi::ftp_server_pi::_handle(SOCKET sockid, ClientInf *client_inf) {
    char *str;
    char *args;
    int cmd;
    while (str = client_inf->m_cmd_buffer.pop(), str) {
        printf("%s\n", str);
        args = str;
        cmd = ftp_cmds::cmd_dispatch(&args);

        if ((ftp_cmds::FTP_CMDS_INF[cmd].m_need_args == ftp_cmds::FCNA_MANDATORY && args == NULL) || (ftp_cmds::FTP_CMDS_INF[cmd].m_need_args == ftp_cmds::FCNA_NONE && args)) {
            _ftp_send(sockid, "501 Syntax error in parameters or arguments.\r\n");
            continue;
        }

        //if ((_Cmd != FTP_CMD_USER&&_Cmd != FTP_CMD_PASS) && client_inf->m_Status == CLS_CONNECTED) {
        //	_FtpSend(_Socket, "530 Please login with USER and PASS.\r\n");
        //	continue;
        //}

        m_cmd_handler[cmd](this, sockid, client_inf, args);
    }

    return false;
}

bool ftp_server_pi::ftp_server_pi::_ftp_send(SOCKET sockid, const char *buffer) {
    return send(sockid, buffer, strlen(buffer));
}

void ftp_server_pi::ftp_server_pi::cmd_handler_USER(SOCKET sockid, ClientInf *client_inf, char *args) {
    switch (client_inf->m_status) {
    case CLS_CONNECTED:
        client_inf->m_status = CLS_USRNAME_SPECIFIED;
        memcpy(client_inf->m_user_name, args, min(DEFAULT_USRNAME_BUFFER_LEN, strlen(args)));
        _ftp_send(sockid, "331 User name ok,need password.\r\n");
        break;
    case CLS_USRNAME_SPECIFIED:
        memcpy(client_inf->m_user_name, args, min(DEFAULT_USRNAME_BUFFER_LEN, strlen(args)));
        _ftp_send(sockid, "331 User name ok,need password.\r\n");
        break;
    case CLS_PASSWORD_SPECIFIED:
        client_inf->m_status = CLS_USRNAME_SPECIFIED;
        _ftp_send(sockid, "331 User name ok,need password.\r\n");//TODO
        break;
    default:
        assert(false);
        break;
    }
}

void ftp_server_pi::ftp_server_pi::cmd_handler_PASS(SOCKET sockid, ClientInf *client_inf, char *args) {
    switch (client_inf->m_status) {
    case CLS_CONNECTED:
        _ftp_send(sockid, "503 Need account for login.\r\n");
        break;
    case CLS_USRNAME_SPECIFIED:
        client_inf->m_status = CLS_PASSWORD_SPECIFIED;
        memcpy(client_inf->m_passwd, args, min(DEFAULT_PASSWD_BUFFER_LEN, strlen(args)));
        _ftp_send(sockid, "230 User logged in, proceed.\r\n");
        break;
    case CLS_PASSWORD_SPECIFIED:
        _ftp_send(sockid, "230 User logged in, proceed.\r\n");//TODO
        break;
    default:
        assert(false);
        break;
    }
}

void ftp_server_pi::ftp_server_pi::cmd_handler_CWD(SOCKET sockid, ClientInf *client_inf, char *args) {
    _ftp_send(sockid, "500 CWD.\r\n");
}

void ftp_server_pi::ftp_server_pi::cmd_handler_PORT(SOCKET sockid, ClientInf *client_inf, char *args) {
    unsigned long _Port = std::atoi(args);
    if (_Port > 1023) {
        _ftp_send(sockid, "200 Port command successful.\r\n");
    } else {
        _ftp_send(sockid, "500 Port command faild.\r\n");
    }
}

void ftp_server_pi::ftp_server_pi::cmd_handler_PASV(SOCKET sockid, ClientInf *client_inf, char *args) {
    _ftp_send(sockid, "500 PASV.\r\n");
}

void ftp_server_pi::ftp_server_pi::cmd_handler_RETR(SOCKET sockid, ClientInf *client_inf, char *args) {
    _ftp_send(sockid, "500 RETR.\r\n");
    
}

void ftp_server_pi::ftp_server_pi::cmd_handler_STOR(SOCKET sockid, ClientInf *client_inf, char *args) {
    _ftp_send(sockid, "500 STOR.\r\n");
}

void ftp_server_pi::ftp_server_pi::cmd_handler_DELE(SOCKET sockid, ClientInf *client_inf, char *args) {
}

void ftp_server_pi::ftp_server_pi::cmd_handler_RMD(SOCKET sockid, ClientInf *client_inf, char *args) {
}

void ftp_server_pi::ftp_server_pi::cmd_handler_MKD(SOCKET sockid, ClientInf *client_inf, char *args) {
}

void ftp_server_pi::ftp_server_pi::cmd_handler_PWD(SOCKET sockid, ClientInf *client_inf, char *args) {
}

void ftp_server_pi::ftp_server_pi::cmd_handler_LIST(SOCKET sockid, ClientInf *client_inf, char *args) {
}

void ftp_server_pi::ftp_server_pi::cmd_handler_HELP(SOCKET sockid, ClientInf *client_inf, char *args) {
    if (args == NULL) {
        _ftp_send(sockid, ftp_cmds::HELP_MSG);
    } else {
        char *_Tmp = args;
        ftp_cmds::FTP_CMDS _Cmd = ftp_cmds::cmd_dispatch(&_Tmp);
        if (_Cmd != ftp_cmds::FTP_CMDS::FTP_CMD_ERR) {
            _ftp_send(sockid, "214 ");
            _ftp_send(sockid, ftp_cmds::FTP_CMDS_INF[_Cmd].m_help_msg);
            _ftp_send(sockid, "\r\n");
        } else {
            _ftp_send(sockid, "501 Unkown command \"");
            _ftp_send(sockid, args);
            _ftp_send(sockid, "\"\r\n");
        }
    }
}

void ftp_server_pi::ftp_server_pi::cmd_handler_NOOP(SOCKET sockid, ClientInf *client_inf, char *args) {
    _ftp_send(sockid, "200 ok\r\n");
}

void ftp_server_pi::ftp_server_pi::cmd_handler_ERR(SOCKET sockid, ClientInf *client_inf, char *args) {
    _ftp_send(sockid, "500 Syntax error, command unrecognized.\r\n");
}

void ftp_server_pi::ftp_server_pi::cmd_handler_NOT_IMPLEMENTED(SOCKET sockid, ClientInf *client_inf, char *args) {
    _ftp_send(sockid, "502 Command not implemented.\r\n");
}
