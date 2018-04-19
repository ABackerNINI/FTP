#pragma once

#ifndef _NINI_FTP_SERVER_PI_H_
#define _NINI_FTP_SERVER_PI_H_

#include "../ftp_dtp/ftp_dtp.h"
#include "../../resource/ftp_cmds/ftp_cmds.h"
#include "../../../common/common.h"
#include "../../../utility/network/network.h"
#include "../../../utility/string_buffer/string_buffer.h"

namespace ftp_server_pi {

#define DEFAULT_BUFFER_LEN 1024
#define DEFAULT_USRNAME_BUFFER_LEN 100
#define DEFAULT_PASSWD_BUFFER_LEN 100
#define DEFAULT_DIR_BUFFER_LEN 100

#define _CMD(CMD)  network::pointer_cast<cmd_handler>(&ftp_server_pi::cmd_handler_##CMD)

    enum CLIENT_LOGIN_STATUS {
        CLS_CONNECTED,
        CLS_USRNAME_SPECIFIED,
        CLS_PASSWORD_SPECIFIED //Login Successfully
    };

    struct ClientInf {
        //TODO seperate user&passwd with others
        char	                m_user_name[100];
        char	                m_passwd[100];
        char	                m_working_dir[100];
        bool	                m_is_pasv;
        int		                m_port;
        unsigned long           m_ip;
        CLIENT_LOGIN_STATUS     m_status;
        string_buffer           m_cmd_buffer;
        ftp_dtp::ftp_dtp        m_dtp;

        ClientInf();
    };

    class ftp_server_pi :public network::Server {
    public:
        ftp_server_pi();

    protected:
        void on_accepted(network::SVR_SOCKET_CONTEXT *sock_ctx) override;

        void on_recvd(network::SVR_SOCKET_CONTEXT *sock_ctx) override;

        void on_sent(network::SVR_SOCKET_CONTEXT *sock_ctx) override;

        void on_closed(network::SVR_SOCKET_CONTEXT *sock_ctx) override;

        bool _handle(SOCKET sockid, ClientInf *client_inf);

        bool _ftp_send(SOCKET sockid, const char *buffer);

    protected:
        typedef void(*cmd_handler)(ftp_server_pi*, SOCKET, ClientInf*, char *);

        const cmd_handler m_cmd_handler[ftp_cmds::FTP_CMDS_NUM + 1] = {//Need to Handle FTP_CMD_ERR
            { _CMD(USER)},
            { _CMD(PASS)},
            { _CMD(NOT_IMPLEMENTED)},//ACCT
            { _CMD(CWD)},
            { _CMD(NOT_IMPLEMENTED)},//CDUP
            { _CMD(NOT_IMPLEMENTED)},//SMNT
            { _CMD(NOT_IMPLEMENTED)},//QUIT
            { _CMD(NOT_IMPLEMENTED)},//REIN
            { _CMD(PORT)},//PORT
            { _CMD(PASV)},
            { _CMD(NOT_IMPLEMENTED)},//TYPE
            { _CMD(NOT_IMPLEMENTED)},//STRU
            { _CMD(NOT_IMPLEMENTED)},//MODE
            { _CMD(RETR)},
            { _CMD(STOR)},
            { _CMD(NOT_IMPLEMENTED)},//STOU
            { _CMD(NOT_IMPLEMENTED)},//APPE
            { _CMD(NOT_IMPLEMENTED)},//ALLO
            { _CMD(NOT_IMPLEMENTED)},//REST
            { _CMD(NOT_IMPLEMENTED)},//RNFR
            { _CMD(NOT_IMPLEMENTED)},//RNTO
            { _CMD(NOT_IMPLEMENTED)},//ABOR
            { _CMD(NOT_IMPLEMENTED)},//DELE
            { _CMD(NOT_IMPLEMENTED)},//RMD
            { _CMD(NOT_IMPLEMENTED)},//MKD
            { _CMD(NOT_IMPLEMENTED)},//PWD
            { _CMD(NOT_IMPLEMENTED)},//LIST
            { _CMD(NOT_IMPLEMENTED)},//NLST
            { _CMD(NOT_IMPLEMENTED)},//SITE
            { _CMD(NOT_IMPLEMENTED)},//SYST
            { _CMD(NOT_IMPLEMENTED)},//STAT
            { _CMD(HELP)},
            { _CMD(NOOP)},
            { _CMD(ERR)}
        };

        void cmd_handler_USER(SOCKET sockid, ClientInf *client_inf, char *args);

        void cmd_handler_PASS(SOCKET sockid, ClientInf *client_inf, char *args);

        void cmd_handler_CWD(SOCKET sockid, ClientInf *client_inf, char *args);

        void cmd_handler_PORT(SOCKET sockid, ClientInf *client_inf, char *args);

        void cmd_handler_PASV(SOCKET sockid, ClientInf *client_inf, char *args);

        void cmd_handler_RETR(SOCKET sockid, ClientInf *client_inf, char *args);

        void cmd_handler_STOR(SOCKET sockid, ClientInf *client_inf, char *args);

        void cmd_handler_DELE(SOCKET sockid, ClientInf *client_inf, char *args);

        void cmd_handler_RMD(SOCKET sockid, ClientInf *client_inf, char *args);

        void cmd_handler_MKD(SOCKET sockid, ClientInf *client_inf, char *args);

        void cmd_handler_PWD(SOCKET sockid, ClientInf *client_inf, char *args);

        void cmd_handler_LIST(SOCKET sockid, ClientInf *client_inf, char *args);

        void cmd_handler_HELP(SOCKET sockid, ClientInf *client_inf, char *args);

        void cmd_handler_NOOP(SOCKET sockid, ClientInf *client_inf, char *args);

        void cmd_handler_ERR(SOCKET sockid, ClientInf *client_inf, char *args);

        void cmd_handler_NOT_IMPLEMENTED(SOCKET sockid, ClientInf *client_inf, char *args);
    };
}

#endif //_NINI_FTP_SERVER_PI_H_