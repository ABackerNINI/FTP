#pragma once

#ifndef _NINI_FTP_SERVER_PI_H_
#define _NINI_FTP_SERVER_PI_H_

#include "../ftp_dtp/ftp_dtp_server.h"
#include "../../resource/ftp_cmds/ftp_cmds.h"
#include "../../../common/common.h"
#include "../../../utility/network/network.h"
#include "../../resource/ftp_cmd_buffer/ftp_cmd_buffer.h"

namespace ftp_server_pi {

#define DEFAULT_BUFFER_LEN 1024
#define DEFAULT_USRNAME_BUFFER_LEN 100
#define DEFAULT_PASSWD_BUFFER_LEN 100
#define DEFAULT_DIR_BUFFER_LEN 500

#define _CMD(CMD)  network::pointer_cast<cmd_handler>(&FtpServerPi::cmd_handler_##CMD)

    enum CLIENT_LOGIN_STATUS {
        CLS_CONNECTED,
        CLS_USRNAME_SPECIFIED,
        CLS_PASSWORD_SPECIFIED //Login Successfully
    };

    struct UserInf {
        //TODO seperate user&passwd with others
        char	                m_user_name[DEFAULT_USRNAME_BUFFER_LEN];
        char	                m_passwd[DEFAULT_PASSWD_BUFFER_LEN];
        char	                m_working_dir[DEFAULT_DIR_BUFFER_LEN];
        bool	                m_is_passive;
        ftp_cmd_buffer          m_cmd_buffer;
        ftp_dtp::FtpDtp         m_dtp;
        CLIENT_LOGIN_STATUS     m_status;

        UserInf();
    };

    class FtpServerPi :public network::Server {
    public:
        FtpServerPi();

    protected:
        void event_handler(network::SVR_SOCKET_CONTEXT *sock_ctx, int ev) override;

        void _on_accepted(network::SVR_SOCKET_CONTEXT *sock_ctx);

        void _on_recvd(network::SVR_SOCKET_CONTEXT *sock_ctx);

        void _on_sent(network::SVR_SOCKET_CONTEXT *sock_ctx);

        void _on_closed(network::SVR_SOCKET_CONTEXT *sock_ctx);

        void _on_file_transfer_complete(network::SVR_SOCKET_CONTEXT *sock_ctx);

        bool _handle(SOCKET sockid, UserInf *user_inf);

        bool _ftp_send(SOCKET sockid, const char *buffer);

    protected:
        typedef void(*cmd_handler)(FtpServerPi*, SOCKET, UserInf*, char *);

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

        void cmd_handler_USER(SOCKET sockid, UserInf *user_inf, char *args);

        void cmd_handler_PASS(SOCKET sockid, UserInf *user_inf, char *args);

        void cmd_handler_CWD(SOCKET sockid, UserInf *user_inf, char *args);

        void cmd_handler_PORT(SOCKET sockid, UserInf *user_inf, char *args);

        void cmd_handler_PASV(SOCKET sockid, UserInf *user_inf, char *args);

        void cmd_handler_RETR(SOCKET sockid, UserInf *user_inf, char *args);

        void cmd_handler_STOR(SOCKET sockid, UserInf *user_inf, char *args);

        void cmd_handler_DELE(SOCKET sockid, UserInf *user_inf, char *args);

        void cmd_handler_RMD(SOCKET sockid, UserInf *user_inf, char *args);

        void cmd_handler_MKD(SOCKET sockid, UserInf *user_inf, char *args);

        void cmd_handler_PWD(SOCKET sockid, UserInf *user_inf, char *args);

        void cmd_handler_LIST(SOCKET sockid, UserInf *user_inf, char *args);

        void cmd_handler_HELP(SOCKET sockid, UserInf *user_inf, char *args);

        void cmd_handler_NOOP(SOCKET sockid, UserInf *user_inf, char *args);

        void cmd_handler_ERR(SOCKET sockid, UserInf *user_inf, char *args);

        void cmd_handler_NOT_IMPLEMENTED(SOCKET sockid, UserInf *user_inf, char *args);
    };
}

#endif //_NINI_FTP_SERVER_PI_H_