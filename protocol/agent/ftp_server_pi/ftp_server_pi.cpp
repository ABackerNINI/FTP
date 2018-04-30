#include "ftp_server_pi.h"
#include <assert.h>
#include <string>

/*
 *  ClientInf
 */
ftp_server_pi::ClientInf::ClientInf() :
    m_is_pasv(false),
    m_port(0),
    m_status(CLS_CONNECTED) {
    m_user_name[0] = '\0';
    m_passwd[0] = '\0';
    m_working_dir[0] = '/';
    m_working_dir[1] = '\0';

    m_dtp.set_passive(true);
}

/*
 *  ftp_server_pi
 */
ftp_server_pi::ftp_server_pi::ftp_server_pi() :network::Server() {
}

void ftp_server_pi::ftp_server_pi::event_handler(network::SVR_SOCKET_CONTEXT *sock_ctx, int ev) {
    switch (ev) {
    case EVENT_ACCEPTED:
        _on_accepted(sock_ctx);
        break;
    case EVENT_RECEIVED:
        _on_recvd(sock_ctx);
        break;
    case EVENT_SENT:
        _on_sent(sock_ctx);
        break;
    case EVENT_CLOSED:
        _on_closed(sock_ctx);
        break;
    case EVENT_USER_FIRST:
        _on_file_transfer_complete(sock_ctx);
        break;
    default:
        assert(false);
        break;
    }
}

void ftp_server_pi::ftp_server_pi::_on_accepted(network::SVR_SOCKET_CONTEXT *sock_ctx) {
    ClientInf *client_inf = new ClientInf();

    client_inf->m_ip = sock_ctx->m_client_addr.sin_addr.S_un.S_addr;

    _ftp_send(sock_ctx->m_client_sockid, "220 Welcome to NINI's FTP service.\r\n");

    if (sock_ctx->m_bytes_transferred > 0) {
        client_inf->m_cmd_buffer.push(sock_ctx->m_buffer, sock_ctx->m_bytes_transferred);

        _handle(sock_ctx->m_client_sockid, client_inf);
    }

    sock_ctx->m_extra = client_inf;
}

void ftp_server_pi::ftp_server_pi::_on_recvd(network::SVR_SOCKET_CONTEXT *sock_ctx) {
    ClientInf *client_inf = (ClientInf *)(sock_ctx->m_extra);

    client_inf->m_cmd_buffer.push(sock_ctx->m_buffer, sock_ctx->m_bytes_transferred);

    _handle(sock_ctx->m_client_sockid, client_inf);
}

void ftp_server_pi::ftp_server_pi::_on_sent(network::SVR_SOCKET_CONTEXT *sock_ctx) {
}

void ftp_server_pi::ftp_server_pi::_on_closed(network::SVR_SOCKET_CONTEXT *sock_ctx) {
    delete (ClientInf *)(sock_ctx->m_extra);
}

void ftp_server_pi::ftp_server_pi::_on_file_transfer_complete(network::SVR_SOCKET_CONTEXT *sock_ctx) {
    _ftp_send(*(SOCKET *)sock_ctx, "500 File transfer complete.\r\n");
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
    client_inf->m_dtp.set_passive(true);
}

void ftp_server_pi::ftp_server_pi::cmd_handler_RETR(SOCKET sockid, ClientInf *client_inf, char *args) {
    _ftp_send(sockid, "500 RETR.\r\n");    
    
    client_inf->m_dtp.set_is_to_send(true);
    client_inf->m_dtp.set_port(20);
    client_inf->m_dtp.set_fpath(args);
    client_inf->m_dtp.set_completion_port_2(m_completion_port);
    client_inf->m_dtp.set_sockid_2(sockid);

    client_inf->m_dtp.start();
}

void ftp_server_pi::ftp_server_pi::cmd_handler_STOR(SOCKET sockid, ClientInf *client_inf, char *args) {
    _ftp_send(sockid, "500 STOR.\r\n");    

    client_inf->m_dtp.set_is_to_send(false);
    client_inf->m_dtp.set_port(20);
    client_inf->m_dtp.set_fpath(args);
    client_inf->m_dtp.set_completion_port_2(m_completion_port);
    client_inf->m_dtp.set_sockid_2(sockid);

    client_inf->m_dtp.start();
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
